package hack.udc.mjmj.clientesat.utilities;

import javafx.scene.input.KeyCode;
import javafx.scene.input.KeyEvent;

public final class DoomKeyMapper {

    private DoomKeyMapper() {}

    //Mappeo de KeyEvent a Doom Key (lo que procesa el server)

    //Constantes de las distintas teclas
    public static final byte KEY_RIGHTARROW = (byte) 0xae;
    public static final byte KEY_LEFTARROW  = (byte) 0xac;
    public static final byte KEY_UPARROW    = (byte) 0xad;
    public static final byte KEY_DOWNARROW  = (byte) 0xaf;

    public static final byte KEY_USE        = (byte) 0xa2;
    public static final byte KEY_FIRE       = (byte) 0xa3;

    public static final byte KEY_ESCAPE     = 27;
    public static final byte KEY_ENTER      = 13;

    public static final byte KEY_EQUALS     = 0x3d;
    public static final byte KEY_MINUS      = 0x2d;

    public static final byte KEY_RSHIFT     = (byte) (0x80 + 0x36); // 0xb6
    public static final byte KEY_LALT       = (byte) (0x80 + 0x38); // 0xb8

    public static final byte KEY_F2  = (byte) (0x80 + 0x3c);
    public static final byte KEY_F3  = (byte) (0x80 + 0x3d);
    public static final byte KEY_F4  = (byte) (0x80 + 0x3e);
    public static final byte KEY_F5  = (byte) (0x80 + 0x3f);
    public static final byte KEY_F6  = (byte) (0x80 + 0x40);
    public static final byte KEY_F7  = (byte) (0x80 + 0x41);
    public static final byte KEY_F8  = (byte) (0x80 + 0x42);
    public static final byte KEY_F9  = (byte) (0x80 + 0x43);
    public static final byte KEY_F10 = (byte) (0x80 + 0x44);
    public static final byte KEY_F11 = (byte) (0x80 + 0x57);

    // ========================================================

    /**
     * Función para los casteos de KeyEvent a Doom Key
     * Devuelve 0 si la tecla no es válida.
     */
    public static byte convert(KeyEvent event) {

        KeyCode code = event.getCode();

        switch (code) {

            case ENTER:
                return KEY_ENTER;

            case ESCAPE:
                return KEY_ESCAPE;

            case LEFT:
                return KEY_LEFTARROW;

            case RIGHT:
                return KEY_RIGHTARROW;

            case UP:
                return KEY_UPARROW;

            case DOWN:
                return KEY_DOWNARROW;

            case SPACE:
                return KEY_USE;

            case CONTROL:
                return KEY_FIRE;

            case SHIFT:
                return KEY_RSHIFT;

            case ALT:
                return KEY_LALT;

            case F2:
                return KEY_F2;
            case F3:
                return KEY_F3;
            case F4:
                return KEY_F4;
            case F5:
                return KEY_F5;
            case F6:
                return KEY_F6;
            case F7:
                return KEY_F7;
            case F8:
                return KEY_F8;
            case F9:
                return KEY_F9;
            case F10:
                return KEY_F10;
            case F11:
                return KEY_F11;

            case EQUALS:
                return KEY_EQUALS;

            case MINUS:
                return KEY_MINUS;

            default:
                return handleAscii(event);
        }
    }

    /**
     * Replica el comportamiento:
     *     key = tolower(key);
     */
    private static byte handleAscii(KeyEvent event) {

        String text = event.getText();

        if (text == null || text.isEmpty()) {
            return 0;
        }

        char c = text.charAt(0);

        // Solo ASCII imprimible
        if (c < 32 || c > 126) {
            return 0;
        }

        return (byte) Character.toLowerCase(c);
    }

    /**
     * Construye paquete de red compacto:
     * [1 byte key][1 byte state]
     */
    public static byte[] buildPacket(KeyEvent event, boolean pressed) {

        byte doomKey = convert(event);

        if (doomKey == 0) {
            return null;
        }

        return new byte[] {
                doomKey,
                (byte) (pressed ? 1 : 0)
        };
    }
}