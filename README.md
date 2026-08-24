# spi-to-can-rp2350

mcp2518fd driver for the rp2350 to enable communication over CAN bus through SPI protocol

## Build and Make configurations

### Connection Diagram
https://drive.google.com/file/d/1ZWddQFBW2MUMJHvnhTNMZumeyX-481jm/view?usp=sharing

### Assumptions

I am writing this guide under the assumption that openocd and the pico environment are set up on your system.
I am also assuming that you are using the two-pico setup, with a pico1 as a hardware debugger and a pico2
as a target board. 

The setup guide for the openocd and pico environment is here, but it's better to just ask Fred about this.
https://docs.google.com/document/d/1_4DQLY6vFT5FQG7_9an4s5RqUcZifFovZtI_F7NQG68/edit?usp=sharing

### Compiling the code
First, clone this repository.

Create a build folder inside this repository. <br> 
`mkdir build`

Navigate into the build directory <br> 
`cd build`

Run the cmake command to build <br>
`cmake -DCMAKE_BUILD_TYPE=Debug -DPICO_SDK_PATH=<path to your pico sdk repository> ..`

Some notes about the above command:
- We are setting the build type to be Debug because we are going to run our program through GDB
- Replace `<path to your pico sdk repistory> `with the path to your pico sdk repository. Do not leave in the '<' and '>'. For example, the command that I run is <br>
`cmake -DCMAKE_BUILD_TYPE=Debug -DPICO_SDK_PATH=../../../pico-sdk ..`
- Do not forget the .. because you are in the build directory

Now make everything
`make`

If you want to redo the build process, just `rm -rf build` and do the cmake and make over again. Be careful of using rm -rf though.

### Running the code
Open two terminal sessions. I will call the first terminal TERM1 and the second terminal TERM2.

In TERM1,it doesn't matter what directory you are in
Start openocd so that you can run GDB on it later <br> 
`sudo openocd -s tcl -f interface/cmsis-dap.cfg -f target/rp2350.cfg -c "adapter speed 5000"`

You will see a bunch of text on your terminal, and something like
"Info : Listening on port 3333 for gdb connections". Depending on your computer, it might not be 
port 3333, but keep in mind the port number.


In TERM2, navigate back to this repository and cd into the build directory you made before. <br> 
`gdb mcp2518fd.elf` <br> 
`set architecture arm` <br> 
`target remote localhost:3333` <br> 
Replace 3333 with whatever port came up for you before.

Turn on the terminal user interface because its easier to read <br> 
`tui enable`

To run the code through gdb as if there were no debugger: <br> 
`break main` <br> 
`continue` <br> 
`continue` <br> 

The first continue gets you to the start of main. The second continue runs the program.


