package hack.udc.mjmj.clientesat.comunicacion;

import javafx.application.Platform;
import javafx.scene.canvas.Canvas;

import java.io.IOException;
import java.net.*;

public class UDPController {

    private int puerto;

    private DatagramSocket socket;
    private boolean encendido;

    private Canvas canvas;

    public UDPController(int puerto, Canvas canvas) {
        this.puerto = puerto;
        this.canvas = canvas;
        encendido = false;
    }

    public void initUDP(){
        encendido = true;
        new Thread(() -> {
            try{
                socket = new DatagramSocket(puerto);
                byte[] buf = new byte[1024];
                DatagramPacket packet = new DatagramPacket(buf, buf.length);

                while(encendido){
                    socket.receive(packet);
                    String mnsj = new String(packet.getData(), 0, packet.getLength());

                    Platform.runLater(() -> renderOutput(mnsj));
                }
            }catch (IOException e){e.printStackTrace();}
        }).start();
    }

    public void renderOutput(String mnsj){

    }
}
