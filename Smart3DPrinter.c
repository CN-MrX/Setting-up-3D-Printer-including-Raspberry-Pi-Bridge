#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <jansson.h>
#include <time.h>
#include <unistd.h>

char apiKey[]="38D2A5008AA646BA8FE889CEC100DC40";

void getUrl(char getCommand[])
{

    //Step1: Get json from printer and write the result to get.json
        char finalGetCommand[1024]={'\0'};
        char localIP[] = "curl -X GET -H 'Content-Type':'application/json' \"127.0.0.1";
        char middle[] = "?apikey=";

        strcat(finalGetCommand,localIP);
        strcat(finalGetCommand,getCommand);
        strcat(finalGetCommand,middle);
        strcat(finalGetCommand,apiKey);
        strcat(finalGetCommand,"\" > get.json");

       // printf("finalcommand = %s\n",finalGetCommand);
        system(finalGetCommand);

    //Step2: Convert json to influxDB line protocol
    
        //Step2_1: Read JSON from file
        FILE *fp;
        fp = fopen("get.json", "r");
        char buffer[1024];
        int len = fread(buffer, 1, 1023, fp); // read up to 1023 bytes
        buffer[len] = '\0'; // add null terminator
        fclose(fp);
    
        //Step2_2: Parse JSON
        json_error_t error;
        json_t *root = json_loads(buffer, 0, &error);
        if(!root) {
            fprintf(stderr, "Error parsing JSON: %s\n", error.text);
            exit(1);
        }
    
        //Step2_3: Get fields from JSON
        json_t *sd = json_object_get(root, "sd");
        json_t *state = json_object_get(root, "state");
        json_t *temperature = json_object_get(root, "temperature");
    
        //Step2_4: Extract data from fields
        int sd_ready = json_boolean_value(json_object_get(sd, "ready"));
        const char *text = json_string_value(json_object_get(state, "text"));
        double bed_actual = json_real_value(json_object_get(json_object_get(temperature, "bed"), "actual"));
        double tool0_actual = json_real_value(json_object_get(json_object_get(temperature, "tool0"), "actual"));
        
        //Step2_5: Output data in InfluxDB line protocol format
    	char sd_ready_result[1024]={'\0'};
    	char state_text_result[1024]={'\0'};
    	char bed_actual_result[1024]={'\0'};
    	char tool0_actual_result[1024]={'\0'};

        sprintf(sd_ready_result,"a,name=sd_ready value=%d", sd_ready);
        sprintf(state_text_result,"a,name=state_text value=%s", text);
        sprintf(tool0_actual_result,"a,name=tool0_actual value=%f", tool0_actual);
        sprintf(bed_actual_result,"a,name=bed_actual value=%f", bed_actual);
	
    	printf("%s\n",sd_ready_result);
    	printf("%s\n",state_text_result);
    	printf("%s\n",tool0_actual_result);
    	printf("%s\n",bed_actual_result);
        
        //Step2_6: Clean up
        json_decref(root);


    //Step3: Send the line protocol to the broker via MQTT

        
	char mqtt_sd_ready_result[1024]={'\0'};
	char mqtt_state_text_result[1024]={'\0'};
	char mqtt_bed_actual_result[1024]={'\0'};
	char mqtt_tool0_actual_result[1024]={'\0'};

	sprintf(mqtt_sd_ready_result,"mosquitto_pub -h 172.31.128.141 -t sf/printer/a -m \"%s\"",sd_ready_result);
	sprintf(mqtt_state_text_result,"mosquitto_pub -h 172.31.128.141 -t sf/printer/a -m \"%s\"",state_text_result);
	sprintf(mqtt_bed_actual_result,"mosquitto_pub -h 172.31.128.141 -t sf/printer/a -m \"%s\"",bed_actual_result);
	sprintf(mqtt_tool0_actual_result,"mosquitto_pub -h 172.31.128.141 -t sf/printer/a -m \"%s\"",tool0_actual_result);

//	printf("%s\n",mqtt_sd_ready_result);
//	printf("%s\n",mqtt_state_text_result);
//	printf("%s\n",mqtt_bed_actual_result);
//	printf("%s\n",mqtt_tool0_actual_result);

	system(mqtt_sd_ready_result);
	system(mqtt_state_text_result);
	system(mqtt_bed_actual_result);
	system(mqtt_tool0_actual_result);
}


int main(int argc, char *argv[])
{   
	while(1){
	    system("clear");
            getUrl("/api/printer");
	    sleep(2);
	}
}
