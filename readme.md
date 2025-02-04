# M5StackC-Plus Cloud Function Execution

A sketch for the [M5Stack M5StickC-Plus](https://shop.m5stack.com/products/m5stickc-plus-esp32-pico-mini-iot-development-kit) battery powered device that triggers a URL (a REST API or cloud function) the press of a button.

The particular project is more about how to trigger a remote URL via TLS (HTTPS) and pass JSON data along with the request. I didn't know how to do this on the ESP32 platform, so I created this project to teach myself. The bonus is I get a mini garage opener out of it too (see Background below).

## Background

Many years ago, I built a remote garage door opener using the [Particle](https://www.particle.io/) platform. Since then, I built multiple mobile apps to open and close my garage door remotely using the Particle Photon project.  For a variety of reasons, I implemented a cloud function between the latest version of the mobile garage door opener; this cloud function provided the app with some additional security for one of its features. The result though, is that I have a cloud function I can trigger to open my garage door no matter where I am in the world.

While recently driving a loaner car, I became frustrated pulling out my phone every time I wanted to open or close the garage door. I'd have to grab my phone, login, locate the app, launch the app, and tap on the screen to open the garage door. I started looking for a simpler solution I could use in this situation and started poking through the [M5Stack](https://m5stack.com/) devices I had laying around in my office.

Looking at the M5StickC PLUS  I realized that I could use this device in place of the mobile app to quickly get my garage door opened or closed from within my car. The M5StackC PLUS offers a power button on the side of the device plus a screen and large button on the device face. It's battery powered and offers dual processor cores plus a battery. With this device, I knew I could build a solution that I could use to remotely trigger not only my garage door opener cloud function, but any public cloud function as well. 

## Operation

With the sketch included in this repository, you configure the code with the remote URL you want triggered (executed) when the user presses the main button on the M5StackC-PLUS device. 

**Note:** in this section, the variable names listed are configuration options described in **Configuring the Sketch**. 

When the user powers on the device, here's what happens:

1. The sketch validates the external configuration file. If any of the required configuration options are missing or empty, the sketch will fill the display with Red, alert you to the configuration error, and do nothing else (until, of course, you fix it).
2. Connects to the Wi-Fi network configured in the configuration file
3. Prompts the user to press the primary button on the device to execute/trigger the configured remote URL. 
4. When the user (you) presses the button, the sketch connects to the remote site and updates the display with success or failure based on what happened. 
4. After 30 seconds (30000 milliseconds, configurable in the config file) of inactivity, the device shuts itself down. 

Use the Arduino IDE and Serial Monitor to view the results of the remote URL connection during testing.

## Configuring the Sketch

Rather than requiring that you modify the project source code to configure the sketch for your environment and particular use case, the sketch looks for a configuration file called `config.h` in the project folder. The repository comes with a file called `config.h.rename`, simply rename that file to `config.h` and enter your configuration settings in the renamed file. 

>  When looking at the configuration file, the `#define` statements define constant values the Arduino compiler uses during compilation. When you modify the file with settings for your particular environment and use case, pay special attention to the format of the configuration settings. String values, like the URL for the remote function, must be placed between the double quote marks. Do not change the style of the quotation marks. Numerical values don't use quotation marks.

Default configuration settings:

```c
#define REMOTE_HOST ""
#define DELAY_FAILURE 10000
#define DELAY_INACTIVITY 30000
#define DELAY_SUCCESS 5000
#define WIFI_SSID ""
#define WIFI_PSWD ""

#define USE_PARAMS false
const int PARAM_ROWS = 2;
String paramsArray[PARAM_ROWS][2] = {
  { "", "" }, { "", "" }
};
```

The following table describes each configuration setting:

| Configuration Value | Description                                                  |
| ------------------- | ------------------------------------------------------------ |
| `REMOTE_HOST`       | Defines the remote cloud function triggered (executed) by the sketch. You must specify a full path pointing to the cloud function. |
| `DELAY_FAILURE`     | Specifies the number of milliseconds the sketch waits after a failure before powering off the device. |
| `DELAY_INACTIVITY`  | Specifies the number of milliseconds of inactivity (no button press) the sketch waits before powering off the device. |
| `DELAY_SUCCESS`     | Specifies the number of milliseconds the sketch waits after a success before powering off the device. |
| `WIFI_SSID`         | The Wi-Fi SSID (Service Set Identifier) for the local Wi-Fi network you want the device connected to. The device cannot trigger a cloud function without access to the a network and the Internet. |
| `WIFI_PSWD`         | The password for the Wi-Fi network specified in `WIFI_SSID`. |
| `USE_PARAMS`        | Specifies whether to send the `paramsArray` values to the remote host with the request. The sketch converts the array into a JSON object for transmission. |
| `PARAM_ROWS         | Set this value to the number of rows in the `paramsArray`. |
| `paramsArray`       | A two dimension string array containing the keys and values sent to the remote host with the request. See below for more details. |

**Note:** When `USE_PARAMS` set to true, the sketch sends a POST request to the remote host. If `false` it sends a GET request. Why? Because if you're not sending any data, why use POST?  If you need to send a POST with no data, you can tweak the sketch to do it.

Here's an example of a properly formatted and populated constants file:

```c
#define REMOTE_HOST "https://someremotehost/somerestapi"
#define DELAY_FAILURE 10000
#define DELAY_INACTIVITY 30000
#define DELAY_SUCCESS 5000
#define WIFI_SSID "MyLocalNetwork"
#define WIFI_PSWD "My complicated and sophisticated Wi-Fi passkey"

#define USE_PARAMS false
const int PARAM_ROWS = 2;
String paramsArray[PARAM_ROWS][2] = {
  { "", "" }, { "", "" }
};
```

With `USE_PARAMS` true, this is what the config looks like:

```c
#define REMOTE_HOST "https://someremotehost/somerestapi"
#define DELAY_FAILURE 10000
#define DELAY_INACTIVITY 30000
#define DELAY_SUCCESS 5000
#define WIFI_SSID "MyLocalNetwork"
#define WIFI_PSWD "My complicated and sophisticated Wi-Fi passkey"

#define USE_PARAMS true
const int PARAM_ROWS = 2;
String paramsArray[PARAM_ROWS][2] = {
  { "param1", "vallue1" }, { "param2", "value2" }
};
```

Populate the array with as many key/value pairs as needed, ensuring that the `PARAM_ROWS` property reflects the number of rows in the array.

## Operation

**Note:** Currently, the sketch indicates success only when the remote server returns a 200 (`HTTP OK`) response code. That should be OK for most use cases, but if you want to respond differently to other response codes, you must modify the sketch yourself.

To run this sketch on a M5Stack M5StickC-Plus device, modify the configuration as described in the previous section, build, then deploy the sketch to the device.

To power on the device, press the button on the left side of the device, it's the one highlighted with the blue arrow in the following figure.

![A photo of the sketch in action on the device](/images/image-01.jpg)

To power the device off, depress the power button for 6 seconds (or so).

To trigger/execute the remote URL configured in the sketch, press the button labeled **M5** immediately below the screen displaying the words **Push Button**.

If execution completes successfully, the screen will turn green and you'll see **Success!** on the display. If it fails for whatever reason, the screen will turn red and display **Failure**.  Use the Arduino IDE Serial Monitor to help you troubleshoot issues that arise when connecting to the remote host.

## FAQ

**Does this sketch work with other M5Stack devices?**

Yes, it should, but I haven't tested it on any other devices. The device you select must have a button (like the M5StickC-Plus does) in order for this to work. You may have to change the button press code to recognize your device's button configuration.

**Can the sketch connect to a local service/endpoint?**

Yes, it should. The local endpoint must use TLS (HTTPS) and, likely, a self-signed certificate. 

***

If this code helps you, please consider buying me a coffee.

<a href="https://www.buymeacoffee.com/johnwargo" target="_blank"><img src="https://cdn.buymeacoffee.com/buttons/default-orange.png" alt="Buy Me A Coffee" height="41" width="174"></a>
