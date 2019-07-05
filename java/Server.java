import java.io.*;
import java.net.*;

public class Server {
  public static void main(String args[]) {
    try {
      int port = Integer.parseInt(args[0]);
      ServerSocket ss = new ServerSocket(port);

      while (true) {
        Socket s = ss.accept();
      
        InputStream is = s.getInputStream();
        DataInputStream dis = new DataInputStream(is);
        int req = dis.readInt();

        OutputStream os = s.getOutputStream();
        DataOutputStream dos = new DataOutputStream(os);
        dos.writeInt(req*req);

        dos.close();
        dis.close();
      }
    }
    catch(Exception e) {
      System.out.println("Expppp" + e);
    }
  }
}
