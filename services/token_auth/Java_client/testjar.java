
import com.caredear.backend.client.SocketClient;
import com.caredear.backend.rs.RequestPayload;
 

public class testjar
{
 
  // call our constructor to start the program
  public static void main(String[] args)
  {
    new testjar();
  }
   
  public testjar()
  {
    String testServerName = "localhost";
    int port = 11222;
    try
    {
     SocketClient sc = new SocketClient(testServerName,port);
     sc.start();
     RequestPayload pl = new RequestPayload("13776670730#com.caredear.xmpp#1403578305#r5O16lD7rQKGldNxpFsCNKQkJL8zcxEroQfDvakciLM9TdAjVlZeIiTfte8QirkGqXpHrBUwOBLkgOcvEPqOKEKf1bu4jI0LxjuQsCSnRTw9oB+CU0eSWjlEXp0/s2gUJR99mFX18FtpCANknnfsLztZviLgr2bg64NYXLRv1CY=");
     String res =sc.sendRequest(pl,10);
     System.out.println("test result:"+res);
     System.exit(0);
    }
    catch (Exception e)
    {
      e.printStackTrace();
       System.out.println("test result:"+e.toString());
       System.exit(0);
    }
  }
   
}
