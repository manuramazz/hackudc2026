package hack.udc.mjmj.clientesat.comunicacion;

import hack.udc.mjmj.clientesat.utilities.DoomKeyMapper;
import hack.udc.mjmj.clientesat.utilities.KeyHandler;
import javafx.scene.Scene;
import javafx.scene.input.KeyCode;

import java.io.BufferedReader;
import java.io.IOException;
import java.io.PrintWriter;
import java.net.*;
import java.util.Arrays;

public class TCPController {

    private final int puerto;

    private final static String ipSat = "10.64.151.20";

    private ServerSocket tcpSocket;
    private Socket satSocket;
    private PrintWriter send;

    private BufferedReader in;

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

                send = new PrintWriter(satSocket.getOutputStream(), false);

                System.out.println("SOCKET TCP INICIADO Y CONEXIÓN ESTABLECIDA CON ÉXITO");

            }catch(IOException e){e.printStackTrace();}
        }).start();
    }

    public void crearListener(Scene scene){
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
    }

    public void stopServer(){
        try {
            if(send!=null)send.close();
            if(satSocket!=null)satSocket.close();
        }catch (IOException e){e.printStackTrace();}
    }
}
