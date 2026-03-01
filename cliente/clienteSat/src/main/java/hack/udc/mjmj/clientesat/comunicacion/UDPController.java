package hack.udc.mjmj.clientesat.comunicacion;

import hack.udc.mjmj.clientesat.CanvasController;
import hack.udc.mjmj.clientesat.utilities.PackageBuffer;
import javafx.application.Platform;
import javafx.scene.canvas.Canvas;
import javafx.scene.canvas.GraphicsContext;
import javafx.scene.image.PixelFormat;
import javafx.scene.image.PixelWriter;
import javafx.scene.image.WritableImage;

import java.io.ByteArrayOutputStream;
import java.io.IOException;
import java.net.*;
import java.nio.ByteBuffer;
import java.nio.ByteOrder;
import java.util.zip.Inflater;


public class UDPController {

    private final int puerto;

    private PackageBuffer activeFrame = null;
    private int lastCompletedFrameId = -1;

    // Contadores de métricas
    private int frameCount = 0;
    private long bytesReceivedInSecond = 0;
    private long lastMetricsTime = System.currentTimeMillis();

    // Valores calculados
    private double fps = 0;
    private double mbps = 0;

    WritableImage image = new WritableImage(160, 100);
    PixelWriter pw = image.getPixelWriter();
    int[] argbBuffer = new int[160 * 100];

    private DatagramSocket socket;
    private DatagramPacket packet;
    private boolean encendido;

    private Canvas canvas;
    private CanvasController controller;
    private boolean elementosOcultos = false;

    public UDPController(int puerto) {
        this.puerto = puerto;
        encendido = false;
    }

    public void setController(CanvasController controller) {
        this.controller = controller;
    }

    public void initUDP(){
        encendido = true;
        new Thread(() -> {
            try{
                socket = new DatagramSocket(puerto);
                packet = new DatagramPacket(new byte[1208], 1208);
                while(encendido){
                    socket.receive(packet);
                    bytesReceivedInSecond += packet.getLength();

                    if (!elementosOcultos && controller != null) {
                        elementosOcultos = true;
                        Platform.runLater(() -> {
                            controller.ocultarElementos();
                        });
                    }
                    renderOutput();
                }

                }
            catch (IOException e){e.printStackTrace();} catch (Exception e) {
                throw new RuntimeException(e);
            }
        }).start();
    }

    public void renderOutput() throws Exception {
        ByteBuffer bb = ByteBuffer.wrap(packet.getData(), 0, packet.getLength());
        bb.order(ByteOrder.BIG_ENDIAN);

        int frameId = bb.getInt();
        int chunkId = bb.getShort() & 0xFFFF;
        int totalChunks = bb.getShort() & 0xFFFF;
        //System.out.println("Frame: "+frameId+" chunk: "+chunkId+" totalChunks: "+totalChunks);
        byte[] payload = new byte[packet.getLength() - 8];
        bb.get(payload);

        // Ignorar frames viejos
        if (frameId < lastCompletedFrameId) {
            return;
        }

        // Si no hay frame activo o llega uno más nuevo
        if (activeFrame == null || frameId != activeFrame.frameId) {

            activeFrame = new PackageBuffer();
            activeFrame.frameId = frameId;
            activeFrame.totalChunks = totalChunks;
            activeFrame.data = new byte[totalChunks][];
            activeFrame.data[chunkId] = payload;
            activeFrame.receivedChunks = 1;
        } else {
            activeFrame.data[chunkId] = payload;
            activeFrame.receivedChunks++;
            if(activeFrame.receivedChunks==totalChunks){

                byte[] fullFrame = assemble(activeFrame);
                activeFrame = null;
                lastCompletedFrameId = frameId;
                // --- CÁLCULO DE MÉTRICAS ---
                frameCount++;
                long currentTime = System.currentTimeMillis();
                long elapsedTime = currentTime - lastMetricsTime;

                if (elapsedTime >= 1000) { // Ha pasado 1 segundo (o un poco más)
                    fps = (frameCount * 1000.0) / elapsedTime;
                    // Convertimos bytes a Megabits por segundo (Mbps) o Megabytes (MB/s)
                    // Usaremos MB/s que es más intuitivo para archivos/red
                    mbps = (bytesReceivedInSecond / 1024.0 / 1024.0) / (elapsedTime / 1000.0);

                    // Enviamos al controller (IMPORTANTE: final para usar en el lambda)
                    final double finalFps = fps;
                    final double finalMbps = mbps;

                    if (controller != null) {
                        Platform.runLater(() -> {
                            controller.updateLabels(finalFps, finalMbps);
                        });
                    }

                    // Reiniciamos contadores para el siguiente segundo
                    frameCount = 0;
                    bytesReceivedInSecond = 0;
                    lastMetricsTime = currentTime;
                }
                processCompleteFrame(fullFrame);
            }
        }
    }

    private byte[] assemble(PackageBuffer frame) throws IOException {

        ByteArrayOutputStream baos = new ByteArrayOutputStream();

        for (int i = 0; i < frame.receivedChunks; i++) {
            baos.write(frame.data[i]);
        }

        return baos.toByteArray();
    }


    private void processCompleteFrame(byte[] compressed) throws Exception {

        Inflater inflater = new Inflater();
        inflater.setInput(compressed);

        ByteArrayOutputStream out = new ByteArrayOutputStream();
        byte[] buffer = new byte[160*100];

        while (!inflater.finished()) {
            int count = inflater.inflate(buffer);
            out.write(buffer, 0, count);
        }

        inflater.end();

        byte[] decompressed = out.toByteArray();

        if (decompressed.length != 160 * 100) {
            System.out.println("Paquete corrupto tam = "+decompressed.length);

            return; // frame corrupto
        }

        Platform.runLater(() -> render(decompressed));
    }




    private void render(byte[] frame) {

        for (int i = 0; i < argbBuffer.length; i++) {

            int pixel = frame[i] & 0xFF;

            int intensity = (pixel >> 6) & 0b11;
            int r = (pixel >> 4) & 0b11;
            int g = (pixel >> 2) & 0b11;
            int b = pixel & 0b11;

            int scale = 85;

            r *= scale;
            g *= scale;
            b *= scale;

            double factor = (intensity + 1) / 4.0;

            r = (int)(r * factor);
            g = (int)(g * factor);
            b = (int)(b * factor);

            argbBuffer[i] =
                    (0xFF << 24) |
                            (r << 16) |
                            (g << 8)  |
                            b;
        }

        pw.setPixels(
                0, 0,
                160, 100,
                PixelFormat.getIntArgbInstance(),
                argbBuffer,
                0,
                160
        );

        GraphicsContext gc = canvas.getGraphicsContext2D();
        gc.drawImage(image, 0, 0, 160 * 4, 100 * 4);
    }


    public void setCanvas(Canvas canvas) {
        this.canvas = canvas;
    }
}
