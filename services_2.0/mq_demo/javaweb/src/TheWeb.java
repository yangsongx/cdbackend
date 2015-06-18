import java.io.IOException;
import java.util.concurrent.TimeoutException;

import com.rabbitmq.client.AMQP.BasicProperties;
import com.rabbitmq.client.Connection;
import com.rabbitmq.client.Channel;
import com.rabbitmq.client.ConnectionFactory;
import com.rabbitmq.client.QueueingConsumer;


public class TheWeb{
    public static final int m_mqPort = 5672;
    public static final String m_exchangeName = "amq.direct";
    public static final String m_routeKey = "test";
    public static final String m_reqQueue = "a22301";
    public static final String m_replyQueue = "a22301-back";
    public static String replyQueueName;
    
    public static void main(String argv[]){
        System.out.println("Entering the Java world");
        
        ConnectionFactory factory = new ConnectionFactory();
        
        factory.setHost("localhost");
        factory.setPort(m_mqPort);
        
        try {
            Connection conn = factory.newConnection();
            Channel channel = conn.createChannel();
            //
            channel.exchangeDeclare(m_exchangeName, "direct", true);
            //channel.
            String queueName = channel.queueDeclare().getQueue();
            System.out.println("the que name:" + queueName);
            channel.queueBind(queueName, m_exchangeName, "test");
            channel.queueBind(m_replyQueue, m_exchangeName, "test");
            
            //replyQueue
            
            // FIXME - need specify a new queu
            //QueueingConsumer consumer = new QueueingConsumer(channel);
            //channel.basicConsume(m_replyQueue, false, consumer);
            
            byte[] messageBodyBytes = "Hello, world!".getBytes();
            BasicProperties props = new BasicProperties
                    .Builder()
                    .correlationId("1")
                    .replyTo(m_replyQueue)
                    .build();
            
            channel.basicPublish(m_exchangeName, "test", props, messageBodyBytes);
            
           // channel.basicPublish("", m_reqQueue, props, messageBodyBytes);
            
            messageBodyBytes = "Second req".getBytes();
            BasicProperties props2 = new BasicProperties
                    .Builder()
                    .correlationId("1")
                    .replyTo(m_replyQueue)
                    .build();
            channel.basicPublish(m_exchangeName, "test", null, messageBodyBytes);
            //channel.basicPublish("", m_reqQueue, props2, messageBodyBytes);
            /*
            while(true){
                // waiting for the response
                //QueueingConsumer.Delivery delivery = consumer.nextDelivery();
            }
            */
            
            System.out.println("finished sending");
            
            //conn.close();
            //channel.close();
        } catch (IOException | TimeoutException e) {
            // TODO Auto-generated catch block
            e.printStackTrace();
        }
        
        
    }
}