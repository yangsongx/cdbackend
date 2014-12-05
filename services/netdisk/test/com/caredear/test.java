package com.caredear;

import java.io.*;
import java.net.*;

import com.google.protobuf.*;
import com.caredear.NetdiskMessage;

public class test{

    private static int byte2int(byte[] b) {
        int i = 0;
        i = (((b[3] & 0xff) << 24) | ((b[2] & 0xff) << 16) | ((b[1] & 0xff) << 8)
                | (b[0] & 0xff));
        return i;
    }

    private static short byte2short(byte[] b) {

        short i = 0;

        i = (short)( (short)((b[1] & 0xff) << 8) | (short)(b[0] & 0xff) );

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

	// only take 2 bytes of int
    private static byte[] short2byte(int res) {
        byte[] targets = new byte[2];
        targets[0] = (byte) (res & 0xff);
        targets[1] = (byte) ((res >> 8) & 0xff);

        return targets;
    }

	public static void main(String []args) {
		int target_port = 12001;
		String target_ip = "127.0.0.1"; // default
		if(args.length > 0) {
			target_ip = args[0];
		}

		System.out.println("Try connecting to " + target_ip);

		try{
			Socket s = new Socket(target_ip, target_port);
			System.out.println("... Connected to the server[OK]");

			// Now, simulate a netdisk request to backend(C++) program...
			NetdiskMessage.NetdiskRequest.Builder bd = NetdiskMessage.NetdiskRequest.newBuilder();
			bd.setUser("13815882359");
			bd.setFilename("abc.jpg");
			bd.setFilesize(2018);

			NetdiskMessage.NetdiskRequest reqData = bd.build();
			System.out.println("the serialize size = " + reqData.getSerializedSize());

			byte [] stream_data = reqData.toByteArray();
			int payload_len = stream_data.length;
			System.out.println("the leading len = " + payload_len);

        	byte []leading = short2byte(payload_len);
        	byte [] raw = new byte[payload_len + 2];
        	System.out.println("the totally len = " + (payload_len + 2) +", and raw len = " + raw.length);

        	// java's arraycopy     a --> b,
        	// not like C's  strcpy b <-- a
        	System.arraycopy(leading, 0, raw, 0, 2);
        	System.arraycopy(stream_data, 0, raw, 2, payload_len);

			System.out.println("all data ready, Java ===> C++ ...");
			s.getOutputStream().write(raw);
            System.out.println("Wrote done, waiting for C++'s response...");

            byte []hdr_len = new byte[2];
            s.getInputStream().read(hdr_len);
            short len = byte2short(hdr_len);
            System.out.println("C++ --> Java : I had " + len + " byte data!");
            byte []rawbyte = new byte[len];
            s.getInputStream().read(rawbyte);
            NetdiskMessage.NetdiskResponse response =
                NetdiskMessage.NetdiskResponse.parseFrom(rawbyte);
            System.out.println("Java : Thank you, I got all of them:");
            System.out.println("\n  Result Code = " + response.getResultCode() +
                    ", uploadURL = " + response.getUploadurl() + ", downloadurl = " + response.getDownloadurl()
                    + ", discKey= " + response.getNetdisckey());

            s.close();


		}catch (IOException ex) {
			System.out.println("exception found when connect to server:" + ex.getMessage());
		}

		System.out.println("exit from the test Java program");
	}
}
