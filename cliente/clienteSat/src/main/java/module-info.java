module hack.udc.mjmj.clientesat {
    requires javafx.controls;
    requires javafx.fxml;

    requires java.logging;
    requires java.desktop; //para realizar logs de la comunicacion

    opens hack.udc.mjmj.clientesat to javafx.fxml;
    exports hack.udc.mjmj.clientesat;
}