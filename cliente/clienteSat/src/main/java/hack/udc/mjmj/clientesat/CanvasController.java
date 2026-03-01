package hack.udc.mjmj.clientesat;

import hack.udc.mjmj.clientesat.comunicacion.TCPController;
import hack.udc.mjmj.clientesat.comunicacion.UDPController;
import javafx.fxml.FXML;
import javafx.scene.canvas.Canvas;
import javafx.scene.control.Label;
import javafx.scene.image.ImageView;

public class CanvasController {
    @FXML
    private Canvas canvas;

    @FXML
    private ImageView imagen;
    @FXML
    private ImageView imagenLogo;
    @FXML
    private Label label;

    @FXML
    public void initialize() {
        canvas.getGraphicsContext2D().setImageSmoothing(false);
    }

    public Canvas getCanvas() {return canvas;}
    public void ocultarElementos() {
        label.setVisible(false);
        imagen.setVisible(false);
        imagenLogo.setVisible(false);

        // desaparece el espacio que ocupan
        label.setManaged(false);
        imagen.setManaged(false);
        imagenLogo.setManaged(false);
    }

}
