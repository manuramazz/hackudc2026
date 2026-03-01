package hack.udc.mjmj.clientesat.comunicacion;

import hack.udc.mjmj.clientesat.utilities.DoomKeyMapper;
import hack.udc.mjmj.clientesat.utilities.KeyHandler;
import javafx.scene.Scene;
import javafx.scene.input.KeyCode;
import javafx.scene.input.KeyEvent;

import java.io.BufferedReader;
import java.io.IOException;
import java.io.OutputStream;
import java.io.PrintWriter;
import java.net.*;

public class TCPController {

    private final int puerto;

    private final static String ipSat = "10.64.151.20";

    private ServerSocket tcpSocket;
    private Socket satSocket;
    private PrintWriter send;

    private BufferedReader in;

    OutputStream out;

    public  TCPController(int puerto) {
        this.puerto = puerto;
    }

    public void initTCPServer(){
        new Thread(() -> {
            try {
                tcpSocket = new ServerSocket(puerto);
                System.out.println("Servidor TCP escuchando en puerto " + puerto + "...");

                satSocket = tcpSocket.accept();
                System.out.println("Cliente conectado desde " + satSocket.getInetAddress());

                out = satSocket.getOutputStream();
                send = new PrintWriter(out, false);

                System.out.println("SOCKET TCP INICIADO Y CONEXIÓN ESTABLECIDA CON ÉXITO");

            }catch(IOException e){e.printStackTrace();}
        }).start();
    }

    /*public void crearListener(Scene scene){
        scene.setOnKeyPressed(event -> {
            KeyCode keyCode = event.getCode();
            if(KeyHandler.keyStates.get(keyCode)==null || !KeyHandler.keyStates.get(keyCode)){
                KeyHandler.keyStates.put(keyCode, true);
                if (send!=null){
                    send.println(Arrays.toString(DoomKeyMapper.buildPacket(event, KeyHandler.keyStates.get(keyCode))));
                    send.flush();
                    System.out.println(Arrays.toString(DoomKeyMapper.buildPacket(event, KeyHandler.keyStates.get(keyCode))));
                }
            }
        });
        scene.setOnKeyReleased(event -> {
            KeyCode keyCode = event.getCode();
            KeyHandler.keyStates.put(keyCode, false);
            if (send!=null){
                send.println(Arrays.toString(DoomKeyMapper.buildPacket(event, KeyHandler.keyStates.get(keyCode))));
                send.flush();
                System.out.println(Arrays.toString(DoomKeyMapper.buildPacket(event, KeyHandler.keyStates.get(keyCode))));
            }
        });
    }*/

    public void crearListener(Scene scene) {

        scene.setOnKeyPressed(event -> {
            KeyCode keyCode = event.getCode();

            boolean wasDown = Boolean.TRUE.equals(KeyHandler.keyStates.get(keyCode));
            if (!wasDown) {
                KeyHandler.keyStates.put(keyCode, true);
                sendKeyPacket(event, (byte)1);
            }
        });

        scene.setOnKeyReleased(event -> {
            KeyCode keyCode = event.getCode();
            KeyHandler.keyStates.put(keyCode, false);
            sendKeyPacket(event, (byte)0);
        });
    }

    private void sendKeyPacket(KeyEvent event, byte pressed) {
        if (out == null) return;

        try {
            // Esto debe devolverte un byte 0..255 consistente con tu doomkeys mapping
            byte keycode = DoomKeyMapper.convert(event);

            byte[] pkt = new byte[] { keycode, pressed, 0x00 };
            out.write(pkt);
            out.flush();

            // Debug opcional:
            System.out.println("Sent keycode=" + (keycode & 0xFF) + " pressed=" + pressed);

        } catch (IOException e) {
            e.printStackTrace();
            // opcional: out=null; cerrar socket; etc.
        }
    }

    public void stopServer(){
        try {
            if(send!=null)send.close();
            if(satSocket!=null)satSocket.close();
        }catch (IOException e){e.printStackTrace();}
    }
}
