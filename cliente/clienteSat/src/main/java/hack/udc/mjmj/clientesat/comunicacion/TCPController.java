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

    private final static String ipSat = "10.40.206.49";

    private Socket tcpSocket;
    private Socket satSocket;
    private PrintWriter send;

    private BufferedReader in;

    public  TCPController(int puerto) {
        this.puerto = puerto;
    }

    public void initTCPServer(){
        new Thread(() -> {
            try {
                satSocket = new Socket(ipSat,puerto);
                send = new PrintWriter(satSocket.getOutputStream(), true);
                System.out.println("SOCKET TCP INICIADO Y CONECTADO CON EXITO");
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
                    System.out.println(event.getCode().toString() +" (in doom: "+DoomKeyMapper.convert(event)+")");
                }
            }
        });
        scene.setOnKeyReleased(event -> {
            KeyCode keyCode = event.getCode();
            KeyHandler.keyStates.put(keyCode, false);
            if (send!=null){
                send.println(Arrays.toString(DoomKeyMapper.buildPacket(event, KeyHandler.keyStates.get(keyCode))));
                System.out.println(event.getCode().toString() +" (in doom: "+DoomKeyMapper.convert(event)+")");
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
