Instructions:

1. Run: make
2. Install the module using: sudo insmod input_sequence.ko
3. Verify the installation with: sudo dmesg -w // This checks if the module was properly installed
4. Use keyboard: up_arrow, up_arrow, down_arrow, down_arrow, left_arrow, right_arrow, left_arrow, right_arrow, B, A
5. GET JUMPSCARED! or use: /tmp/launch_firefox.sh
6. Remove the module using: sudo rmmod monitor_cpu_usage
7. Verify the removal with: lsmod | grep "input_s" // This checks if the module was properly uninstalled
8. Clean up with: make clean

WARNING: Module needs Firefox installed from APT, AppArmor with proper regulations or using /tmp/launch_firefox.sh
