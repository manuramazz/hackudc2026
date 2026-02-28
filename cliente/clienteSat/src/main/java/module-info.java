module hack.udc.mjmj.clientesat {
    requires javafx.controls;
    requires javafx.fxml;


    opens hack.udc.mjmj.clientesat to javafx.fxml;
    exports hack.udc.mjmj.clientesat;
}