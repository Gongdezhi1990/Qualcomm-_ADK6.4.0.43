// A simple example that demonstrates how to list available devices, connect
// to them and then read a word from data memory.

#include <cstdio>
#include "common.h"
#include "highlevel.h"


static int const AUDIO_SS_ID = 3;
static int const APPS_SS_ID = 4;


static int exit_prompt()
{
    puts("\nDone. Press return.");
    getchar();

    return 0;
}


static void handle_error(ka_err *err, char const *context)
{
    printf("%s error: %s\n", context, err->err_string);
    ka_free_error(err);
}


static void connect_and_read(ka_connection_details const *device)
{
    kalaccess *ka = NULL;
    // Connect using supplied details.
    // May also connect with an MDE-style device URI using ka_connect_with_uri().
    ka_err *err = ka_connect(device, false, &ka);
    if (err)
    {
        handle_error(err, "ka_connect()");
        return;
    }

    // Verify what we connected to.
    unsigned proc_id = 0;
    err = ka_get_processor_id(ka, &proc_id);
    if (err)
    {
        handle_error(err, "ka_get_processor_id()");
        ka_disconnect(ka);
        return;
    }

    printf(
        "Connected to a %s (processor id %u)\n",
        ka_get_chip_name(ka),
        device->processor_id
    );

    uint32_t val = 0;
    err = ka_read_dm(ka, 0, &val);
    if (err)
    {
        if (device->subsys_id == AUDIO_SS_ID)
            printf("\nAudio DM access failed: this is expected if the Audio subsystem is off.\n");

        handle_error(err, "ka_read_dm()");
    }
    else
    {
        printf("Contents of first word of DM is 0x%x\n", val);
    }

    ka_disconnect(ka);
    return;
}


int main()
{
    // Discover supported Kalimba cores on attached devices.
    puts("Discovering attached Kalimba cores...");
    ka_connection_details *devices = NULL;
    int count = 0;
    ka_err *err = ka_trans_build_device_table(&devices, &count); 
    if (err)
    {
        handle_error(err, "ka_trans_build_device_table()");
        return exit_prompt();
    }
    
    ka_connection_details *apps_p1 = NULL;
    ka_connection_details *audio_p0 = NULL;
    for (int i = 0; i < count; ++i)
    {
        printf(
            "    Kalimba core:\n"
            "      transport string: %s\n"
            "      subsystem ID: %d\n"
            "      processor ID: %u\n"
            "      debug dongle name: %s\n",
            devices[i].transport_string,
            devices[i].subsys_id,
            devices[i].processor_id,
            devices[i].dongle_id_string
        );

        if ((devices[i].subsys_id == APPS_SS_ID) && (devices[i].processor_id == 1))
            apps_p1 = &devices[i];
        else if ((devices[i].subsys_id == AUDIO_SS_ID) && (devices[i].processor_id == 0))
            audio_p0 = &devices[i];
    }

    // Demonstrate connection and basic memory reading.

    if (apps_p1)
    {
        puts("\nConnecting to Apps P1...");
        connect_and_read(apps_p1);
    }

    if (audio_p0)
    {
        puts("\nConnecting to Audio P0...");
        connect_and_read(audio_p0);
    }

    if ((count > 0) && !apps_p1 && !audio_p0)
    {
        puts("\nConnecting to the first device...");
        connect_and_read(&devices[0]);
    }
    else if (count == 0)
    {
        puts("\nNo devices found. Ensure that the TRBI200 or USBDebug interface is attached,\n"
             "drivers are installed and the debug interface is unlocked."
        );
    }

    ka_trans_free_device_table(devices, count);
    return exit_prompt();
}
