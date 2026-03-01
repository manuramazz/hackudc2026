package hack.udc.mjmj.clientesat;

import hack.udc.mjmj.clientesat.comunicacion.TCPController;
import hack.udc.mjmj.clientesat.comunicacion.UDPController;
import javafx.fxml.FXML;
import javafx.scene.Scene;
import javafx.scene.canvas.Canvas;
import javafx.scene.control.Label;

public class HelloController {
    @FXML
    private Canvas canvas;

    private UDPController udp;
    private TCPController tcp;

    @FXML
    public void initialize() {

        udp = new UDPController(5001);
        udp.setCanvas(canvas);
        canvas.getGraphicsContext2D().setImageSmoothing(false);
        udp.initUDP();

        tcp = new TCPController(5555);
        tcp.initTCPServer();
    }

    public void setScene(Scene scene) {
        tcp.crearListener(scene);
    }
}
