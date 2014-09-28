
package com.caredear.backend.client;

import java.io.BufferedWriter;
import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStreamWriter;
import java.net.InetAddress;
import java.net.InetSocketAddress;
import java.net.Socket;
import java.net.SocketAddress;
import java.net.SocketTimeoutException;
import java.util.concurrent.Callable;
import java.util.concurrent.ExecutorService;
import java.util.concurrent.Executors;
import java.util.concurrent.FutureTask;
import java.util.concurrent.TimeUnit;
import com.caredear.backend.utils.*;
import com.caredear.backend.rs.RequestPayload;

/**
 * @ClassName: SocketClient
 * @Description: SocketClient
 * @author HeQi heqi@caredear.com
 * @date Jun 29, 2014 10:38:45 PM
 */
public class SocketClient {

    private Socket socket;
    public String address;
    public int port;

    /**
     * @Title: init port and address
     * @Description:
     * @param address
     * @param port
     */
    public SocketClient(String address, int port) {
        this.address = address;
        this.port = port;
    }

    /**
     * @Title: start
     * @Description: open socket
     * @param @throws Exception Setting file
     * @return boolean return type
     * @throws
     */
    public boolean start() throws Exception {
        try {
            socket = openSocket(address, port);
            return true;
        } catch (Exception e) {
            e.printStackTrace();
            throw e;
        }
    }

    /**
     * @Title: sendRequest
     * @Description: send data to server
     * @param @param pl
     * @param @param timeout
     * @param @throws Exception Setting file
     * @return String return type
     * @throws
     */
    public String sendRequest(RequestPayload pl, int timeout) throws Exception {

        final String payload = pl.getPayload();
        ExecutorService executor = Executors.newSingleThreadExecutor();
        FutureTask<String> future =
                new FutureTask<String>(new Callable<String>() {
                    public String call() throws Exception {
                        return writeToAndReadFromSocket(socket, payload);
                    }
                });

        executor.execute(future);
        try {
            String result = future.get(timeout * 1000, TimeUnit.MILLISECONDS);
            socket.close();
            return result;
        } catch (Exception e) {
            e.printStackTrace();
            socket.close();
            throw e;
        }
    }

    private static int byte2int(byte[] b) {

        int i = (((b[3] & 0xff) << 24) | ((b[2] & 0xff) << 16) | ((b[1] & 0xff) << 8)
                | (b[0] & 0xff));

        return i;
    }

    private String writeToAndReadFromSocket(Socket socket, String writeTo) throws Exception {
        try {
            // write to the socket
            BufferedWriter bufferedWriter = new BufferedWriter(new OutputStreamWriter(
                    socket.getOutputStream()));
            bufferedWriter.write(writeTo);
            bufferedWriter.flush();

            // read from the socket
            InputStream is = socket.getInputStream();
            byte[] result = new byte[4];
            if (-1 == is.read(result, 0, 4)) {
                return ErrorCode.SOCKET_ERROR;
            } else {
                if (byte2int(result) == 0) {

                    return ErrorCode.OK;
                } else {

                    return ErrorCode.SOCKET_ERROR;
                }
            }
        } catch (IOException e) {
            e.printStackTrace();
            throw e;
        }
    }

    private Socket openSocket(String server, int port) throws Exception {
        Socket socket;

        // socket timeout
        try {
            InetAddress inteAddress = InetAddress.getByName(server);
            SocketAddress socketAddress = new InetSocketAddress(inteAddress, port);
            // create a socket
            socket = new Socket();
            // this method will block no more than timeout ms.
            int timeoutInMs = 5 * 1000;
            socket.connect(socketAddress, timeoutInMs);

            return socket;
        } catch (SocketTimeoutException ste) {
            ste.printStackTrace();
            throw ste;
        }
    }

}
