package hack.udc.mjmj.clientesat;

import hack.udc.mjmj.clientesat.comunicacion.TCPController;
import hack.udc.mjmj.clientesat.comunicacion.UDPController;
import javafx.application.Application;
import javafx.fxml.FXMLLoader;
import javafx.scene.Scene;
import javafx.stage.Stage;

import java.io.IOException;

public class HelloApplication extends Application {

    private UDPController udp;
    private TCPController tcp;

    @Override
    public void start(Stage stage) throws IOException {
        FXMLLoader fxmlLoader =
                new FXMLLoader(HelloApplication.class.getResource("canvas.fxml"));

        Scene scene = new Scene(fxmlLoader.load(), 320, 240);

        CanvasController controller = fxmlLoader.getController();

        stage.setTitle("DOOM");
        stage.setScene(scene);

        udp = new UDPController(5001);
        udp.setCanvas(controller.getCanvas());
        udp.initUDP();

        tcp = new TCPController(5555);
        tcp.initTCPServer();
        tcp.crearListener(scene);

        stage.show();
    }
}
