package com.caredear;

import java.io.*;
import java.net.*;

import com.google.protobuf.*;
import com.caredear.TokenMessage;

public class TokenMain{
    private static int byte2int(byte[] b) {
        int i = 0;
        i = (((b[3] & 0xff) << 24) | ((b[2] & 0xff) << 16) | ((b[1] & 0xff) << 8)
                | (b[0] & 0xff));
        return i;
    }

    private static byte[] int2byte(int res) {
        byte[] targets = new byte[4];
        targets[0] = (byte) (res & 0xff);
        targets[1] = (byte) ((res >> 8) & 0xff);
        targets[2] = (byte) ((res >> 16) & 0xff);
        targets[3] = (byte) (res >>> 24);

        return targets;
    }

    public static void main(String [] args){
        System.out.println("Try connecting to server socket...");

        //com.google.protobuf.Descriptors.FileDescriptor data = TokenMessage.getDescriptor(); // = new TokenRequest();
        TokenMessage.TokenRequest.Builder b = TokenMessage.TokenRequest.newBuilder();
        b.setUid("13022593515");
        b.setAppid("com.caredear.family");
        b.setLoging("2005-09-07");
        b.setAesString("abcdefg1234567==");

        TokenMessage.TokenRequest data = b.build();
        System.out.println("the serialize size is : " + data.getSerializedSize());

        byte [] stream_data = data.toByteArray();

        int payload_len = stream_data.length;
        System.out.println("the array size is : " + stream_data.length);

        /* adding leading length */
        byte []leading = int2byte(stream_data.length);
        byte [] raw = new byte[payload_len + 4];
        System.out.println("the whole length = " + (payload_len + 4));

        // java's arraycopy     a --> b,
        // not like C's  strcpy b <-- a
        System.arraycopy(leading, 0, raw, 0, 4);
        System.arraycopy(stream_data, 0, raw, 4, payload_len);


        try {
            Socket s = new Socket("127.0.0.1", 11011);

            System.out.println("Connected to Server [OK]");

            // try stream to the C++ side...
            System.out.println("Writing data to server...");
            s.getOutputStream().write(raw);

            System.out.println("Wrote done, waiting for response...");

            // Next will waiting for the response
            //InputStream in = s.getInputStream();

            TokenMessage.TokenResponse response;


            byte [] hdr_len = new byte[4];
            s.getInputStream().read(hdr_len);

            int len = byte2int(hdr_len);
            System.out.println("C++ tell Java there are (" + len + ") bytes data");

            byte [] rawbyte = new byte[len];
            s.getInputStream().read(rawbyte);


            response = TokenMessage.TokenResponse.parseFrom(rawbyte);
            ///
            System.out.println("the result code - " + response.getResultCode());
            if(response.hasExtraCode()){
                System.out.println("error code - " + response.getExtraCode());
            }
            ///

            System.out.println("Closing this socket.\n");
            s.close();

        }catch (IOException e) {
            System.out.println("Exception caught!");
        }

        System.out.println("Exit from main.");
    }
}
