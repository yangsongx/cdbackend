package com.caredear;

import java.io.*;
import java.net.*;

public class test{
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
		}catch (IOException ex) {
			System.out.println("exception found when connect to server:" + ex.getMessage());
		}

		System.out.println("exit from the test Java program");
	}
}
