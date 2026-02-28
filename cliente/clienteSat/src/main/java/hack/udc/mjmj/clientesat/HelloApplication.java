package hack.udc.mjmj.clientesat;

import hack.udc.mjmj.clientesat.comunicacion.TCPController;
import javafx.application.Application;
import javafx.fxml.FXMLLoader;
import javafx.scene.Scene;
import javafx.stage.Stage;

import java.io.IOException;

public class HelloApplication extends Application {
    @Override
    public void start(Stage stage) throws IOException {
        TCPController TCP = new TCPController(5000);
        TCP.initTCPServer();
        FXMLLoader fxmlLoader = new FXMLLoader(HelloApplication.class.getResource("hello-view.fxml"));
        Scene scene = new Scene(fxmlLoader.load(), 320, 240);
        stage.setTitle("Hello!");
        stage.setScene(scene);
        TCP.crearListener(scene);
        stage.show();
    }
}
