# Priority Booster

This driver sample demonstrates communication between a user mode application and a Windows Kernel Driver.

## Installing

You'll need to first install this driver on the target machine. On the target machine, open an elevated command prompt before running these commands.

* To install driver: `sc create booster type= kernel binPath= <absolute path to driver>`
* To start the driver: `sc start booster`
* To stop the driver: `sc stop booster`
* To delete the driver: `sc delete booster`

## Usage

The driver is intended to be used with the PriorityBoosterClient. While the driver is running, you can run PriorityBoosterClient on the target machine with the following command on an elevated command prompt: `PriorityBooster <thread-id> <priority>`. If all goes well, you should see "Success!" printed out.