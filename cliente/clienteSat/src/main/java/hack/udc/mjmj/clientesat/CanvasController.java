package hack.udc.mjmj.clientesat;

import hack.udc.mjmj.clientesat.comunicacion.TCPController;
import hack.udc.mjmj.clientesat.comunicacion.UDPController;
import javafx.fxml.FXML;
import javafx.scene.canvas.Canvas;

public class CanvasController {
    @FXML
    private Canvas canvas;

    @FXML
    public void initialize() {
        canvas.getGraphicsContext2D().setImageSmoothing(false);
    }

    public Canvas getCanvas() {return canvas;}

}
