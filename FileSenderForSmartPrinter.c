#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <jansson.h>

int main(int argc, char *argv[]) {

// Step1: Verify the correct form of input of the program
    // Step1.1: Check if all arguments are provided
    if(argc != 7) {
        printf("Usage: %s -h [IP address] -k [API key] -f [file]\n
            Example:./FileSenderForSmartPrinter -h 172.31.128.136
            -k 38D2A5008AA646BA8FE889CEC100DC40 
            -f ./Batman_0.2mm_PLA_MK3S_23m.gcode\n", argv[0]);
        return -1;
    }

    // Step1.2: Parse command line arguments
    char ip[256];
    char apikey[256];
    char file[256];
    for(int i = 1; i < argc; i += 2) {
        if(strcmp(argv[i], "-h") == 0) {
            strcpy(ip, argv[i+1]);
        } else if(strcmp(argv[i], "-k") == 0) {
            strcpy(apikey, argv[i+1]);
        } else if(strcmp(argv[i], "-f") == 0) {
            strcpy(file, argv[i+1]);
        } else{
            printf("Usage: %s -h [IP address] -k [API key] -f [file]\n
                Example:./FileSenderForSmartPrinter -h 172.31.128.136
                -k 38D2A5008AA646BA8FE889CEC100DC40 
                -f ./Batman_0.2mm_PLA_MK3S_23m.gcode\n", argv[0]);
            return -1;
        }
    }

// Step2: Build curl command and store the result to sendFeedback.json
    char curl_command[1024];
    sprintf(curl_command, "curl -F 'file=@%s' 
        http://%s/api/files/sdcard?apikey=%s >sendFeedback.json",file,ip,apikey);

// Step3: Execute curl command and check if successful
    if(system(curl_command) != 0) {
        printf("Failed to send file to printer.\n");
        return -1;
    } else {
        printf("File successfully sent to printer.\n");

    }

// Step4: Start the terminal to control the printing job
    char job_command[1024]={'\0'};
    char input_command[64]={'\0'};
    while(1)
    {
        // Step4.1 Absorb the input command
        printf(">");
        scanf("%s",input_command);

        // Step4.2 Give feedback or bulid up job_command

        // Case1: Send again
    	if(strcmp(input_command,"resend")==0){
        		if(system(curl_command) != 0) {
            		printf("Failed to send file to printer.\n");
            		return -1;
        		} else {
            		printf("File successfully sent to printer.\n");
    	    	}
    	}
        
        // Case2: Quit the program
    	else if(strcmp(input_command,"quit")==0)
            {
                 break;
            }
        
        // Case3: Start the printing job
    	else if(strcmp(input_command,"start")==0)
            {
                 printf("start\n");

             // 1. Get the target resource
            	// Read JSON from file
            	json_error_t error;
            	json_t *root = json_load_file("sendFeedback.json", 0, &error);
            	if(!root) {
                	fprintf(stderr, "Error parsing JSON: %s\n", error.text);
                	return 1;
            	}

            	 // Get fields from JSON
            	 json_t *files = json_object_get(root, "files");
            	 json_t *local = json_object_get(json_object_get(files, "local"), "refs");
            	 json_t *sdcard = json_object_get(json_object_get(files, "sdcard"), "refs");
     
            	 // Extract data from fields
            	 const char *local_resource = json_string_value(json_object_get(local, "resource"));
            	 const char *sdcard_resource = json_string_value(json_object_get(sdcard, "resource"));
     
            	 // Output data
            	 printf("Local resource: %s\n", local_resource);
            	 printf("SD card resource: %s\n", sdcard_resource);

             // 2. Build up the job_command
     	     sprintf(job_command, "curl -X POST -H 'Content-type':'application/json' 
     	     -d '{\"command\":\"select\",\"print\":true}' %s?apikey=%s",sdcard_resource,apikey);

             // 3. Clean up
        	 json_decref(root);
     	
            // printf("start command:%s\n",job_command);
 
             }

        // Case4: Cancel the printing job
    	else if(strcmp(input_command,"cancel")==0)
        {
	     sprintf(job_command, "curl -X POST -H 'Content-type':'application/json' 
            -d '{\"command\":\"cancel\"}' http://%s/api/job?apikey=%s",ip,apikey);
             //printf("cancel\n");
        }
        
        // Case5: Pause the printing job
        else if(strcmp(input_command,"pause")==0)
        {
         sprintf(job_command, "curl -X POST -H 'Content-type':'application/json' 
           -d '{\"command\":\"pause\",\"action\":\"pause\"}' http://%s/api/job?apikey=%s",ip,apikey);
             //printf("pause\n");
        }

        // Case6: Resume the printing job
        else if(strcmp(input_command,"resume")==0)
        {
         sprintf(job_command, "curl -X POST -H 'Content-type':'application/json'
            -d '{\"command\":\"pause\",\"action\":\"resume\"}' http://%s/api/job?apikey=%s",ip,apikey);
             //printf("resume\n");
        }

        // Case7: Command error
    	else
    	{
    		printf("Command not found!\n");
    		printf("You can use commands: resend, start, cancel, pause, resume or quit\n");
    		continue;
    	}

        // Step4.3 Execute job_command and validate
         if(system(job_command) != 0) {
             printf("Failed to send command to printer.\n");
         } else {
             printf("Command successfully sent to printer.\n");
         }

         
          
         

         }

return 0;
}
