module hack.udc.mjmj.clientesat {
    requires javafx.controls;
    requires javafx.fxml;

    requires java.logging;
    requires java.desktop;

    opens hack.udc.mjmj.clientesat to javafx.fxml;
    exports hack.udc.mjmj.clientesat;
}