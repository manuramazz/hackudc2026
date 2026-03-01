package hack.udc.mjmj.clientesat.utilities;

public class PackageBuffer {
    public int frameId;
    public int totalChunks;
    public byte[][]data;
    public int receivedChunks;

    public PackageBuffer(){
        this.frameId = 0;
        this.totalChunks = 0;
        this.data = null;
        this.receivedChunks = 0;
    }
}
