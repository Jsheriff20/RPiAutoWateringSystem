#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <MQTTClient.h>

#include"plantWateringAppHeader.h"


#define ADDRESS     "54.89.174.250"
#define CLIENTID    "pumpApplication"
#define TOPIC       "moistureChecker"
#define QOS         1
#define TIMEOUT     10000L


gpio_pin apin;
volatile MQTTClient_deliveryToken deliveredtoken;



/*void delivered(void *context, MQTTClient_deliveryToken dt){
	printf("Message with token value %d delivery confirmed\n", dt);
	deliveredtoken = dt;
}*/

int convertMlToDuration(int ml){
	/* convert ml to how long it takes for the pump to output the specified amount of ml
		this calculation is formulated  on the strength of the pump and the length of the plastic tube*/

	/* took 3.6 seconds to fill 100ml */
	/* mlPerSecond = (100 / 3.6) = 27.777...*/
	/* durationForOneMl = 1 / 27.777 = 0.036)  */
	float durationForOneMl = 0.036;


	int secondInMicro = 1000000;
	int duration = ml * (int)(secondInMicro * durationForOneMl);
	printf("\nwatering for %i\n", duration);
	printf("watering %i ml \n", ml);
	return duration;
}




void connLost(void *context, char *cause){
	printf("\nConnection lost\n");
	printf("     cause: %s\n", cause);
}




void startPump(int pin, int value, int duration){
	int fd, ret;
	fd = open("/dev/moistureDev", O_RDWR);
	printf("%i", fd);
	if (fd < 0) {
		printf("Can't open device file: %s\n");
		exit(-1);
	}


	/*copy the string "start pump" to the apin.desc destination */
	strcpy(apin.desc, "toggle pump");

	apin.pin = pin; /* GPIO Number */
	apin.value = value; /* value 0 or 1 */

	/* start */
	printf("\n Starting Pump ");
	ret = ioctl(fd, IOCTL_WRITE, &apin);
	apin.value = !apin.value;
	/* wait */
	printf("\n Waited for %i...", duration);
	usleep(duration);
	/* stop */
	ret = ioctl(fd, IOCTL_WRITE, &apin);
	printf("Stopped pump");

	close(fd);
	printf("Exit 0\n");
}

int msgArrived(void *context, char *topicName, int topicLen,  MQTTClient_message *message){
	char* payloadptr;
	printf("Message arrived from topic; %s", topicName);

	/*get message */
	payloadptr = message->payload;
	int ml = atoi(payloadptr);
	printf("%i", &ml);

	/*convert ml to duration for pump and then start the pump */
	startPump(27, 0, convertMlToDuration(ml));

	/*free memory*/
	MQTTClient_freeMessage(&message);
	MQTTClient_free(topicName);
	return 1;
}



int main(int argc, char* argv[]) {
	printf("User App\n");

	MQTTClient client;
	MQTTClient_connectOptions conn_opts = MQTTClient_connectOptions_initializer;
	MQTTAsync_SSLOptions sslopts = MQTTClient_SSLOptions_initializer;
	int rc;
	int ch;
	
	
	MQTTClient_create(&client, ADDRESS, CLIENTID, MQTTCLIENT_PERSISTENCE_NONE, NULL);
	conn_opts.username = "admin";
	conn_opts.password = "Password123";
	conn_opts.keepAliveInterval = 20;
	conn_opts.cleansession = 1;
	
	
	if(g_mqtt_client.certs_path != NULL ) {                                 

                  conn_opts.ssl = &ssl_opts;                                      
                  conn_opts.ssl->trustStore = "/home/pi/Downloads/certs/ca.crt";   
                  conn_opts.ssl->privateKey = "/home/pi/Downloads/certs/client.key";
                  conn_opts.ssl->keyStore =   "/home/pi/Downloads/certs/client.crt";
                  conn_opts.ssl->ssl_error_cb = ssl_error_cb;                     
                  conn_opts.ssl->enableServerCertAuth = 1;                        
                  conn_opts.ssl->verify = 1;                                      
          }    


	MQTTClient_setCallbacks(client, NULL, connLost, msgArrived, NULL); /* delivered);*/
	if ((rc = MQTTClient_connect(client, &conn_opts)) != MQTTCLIENT_SUCCESS){
		printf("Failed to connect, return code %d\n", rc);
		exit(EXIT_FAILURE);
	}

	printf("Subscribing to topic %s\nfor client %s using QoS%d\n\n Enter Q to quit\n\n", TOPIC, CLIENTID);
	MQTTClient_subscribe(client, TOPIC, QOS);
	do{
		ch = getchar();
	} while(ch!='Q' && ch != 'q');
	MQTTClient_disconnect(client, 10000);
	MQTTClient_destroy(&client);
	return rc;

}