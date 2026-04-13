CC = gcc
CFLAGS = -Wall -Wextra -ICommon
LDFLAGS = -lssl -lcrypto 
CA_PORT ?= 9090
POWER_PORT ?= 12000

# ===== MAKE =====
all: bin/CA_test_port bin/Power_Hypervisor_test \
     bin/Device_1_test bin/Device_2_test bin/Device_3_test

# ===== COMMMON UTILS =====
FILE_UTILS_OBJ = Common/file_utils.o
FILE_UTILS_SRC = Common/file_utils.c
FILE_UTILS_H = Common/file_utils.h

$(FILE_UTILS_OBJ): $(FILE_UTILS_SRC) $(FILE_UTILS_H)
	$(CC) $(CFLAGS) -c $(FILE_UTILS_SRC) -o $(FILE_UTILS_OBJ)

# ===== CA =====
# What it was gcc -Wall -Wextra CA/src/main.c Common/file_utils.c -ICommon -o CA_test_port -lcrypto -lssl
CA_SRC = CA/src/main.c 
CA_OBJ = CA/src/main.o

$(CA_OBJ): $(CA_SRC) $(FILE_UTILS_H)
	$(CC) $(CFLAGS) -c $(CA_SRC) -o $(CA_OBJ)

bin/CA_test_port: $(CA_OBJ) $(FILE_UTILS_OBJ)
	$(CC) $(CA_OBJ) $(FILE_UTILS_OBJ) -o bin/CA_test_port $(LDFLAGS)

# ===== DEVICE 1 =====
DEVICE1_SRC = Firmware_Device_1/src/main.c 
DEVICE1_OBJ = Firmware_Device_1/src/main.o

$(DEVICE1_OBJ): $(DEVICE1_SRC) $(FILE_UTILS_H)
	$(CC) $(CFLAGS) -c $(DEVICE1_SRC) -o $(DEVICE1_OBJ)

bin/Device_1_test: $(DEVICE1_OBJ) $(FILE_UTILS_OBJ)
	$(CC) $(DEVICE1_OBJ) $(FILE_UTILS_OBJ) -o bin/Device_1_test $(LDFLAGS)

# ===== DEVICE 2 =====
DEVICE2_SRC = Firmware_Device_2/src/main.c 
DEVICE2_OBJ = Firmware_Device_2/src/main.o

$(DEVICE2_OBJ): $(DEVICE2_SRC) $(FILE_UTILS_H)
	$(CC) $(CFLAGS) -c $(DEVICE2_SRC) -o $(DEVICE2_OBJ)

bin/Device_2_test: $(DEVICE2_OBJ) $(FILE_UTILS_OBJ)
	$(CC) $(DEVICE2_OBJ) $(FILE_UTILS_OBJ) -o bin/Device_2_test $(LDFLAGS)

# ===== DEVICE 3 =====
DEVICE3_SRC = Firmware_Device_3/src/main.c 
DEVICE3_OBJ = Firmware_Device_3/src/main.o

$(DEVICE3_OBJ): $(DEVICE3_SRC) $(FILE_UTILS_H)
	$(CC) $(CFLAGS) -c $(DEVICE3_SRC) -o $(DEVICE3_OBJ)

bin/Device_3_test: $(DEVICE3_OBJ) $(FILE_UTILS_OBJ)
	$(CC) $(DEVICE3_OBJ) $(FILE_UTILS_OBJ) -o bin/Device_3_test $(LDFLAGS)

# ===== POWER HYPERVISOR =====
POWER_SRC = Power_Hypervisor/src/main.c
POWER_OBJ = Power_Hypervisor/src/main.o 

$(POWER_OBJ): $(POWER_SRC) $(FILE_UTILS_H)
	$(CC) $(CFLAGS) -c $(POWER_SRC) -o $(POWER_OBJ)

bin/Power_Hypervisor_test: $(POWER_OBJ) $(FILE_UTILS_OBJ)
	$(CC) $(POWER_OBJ) $(FILE_UTILS_OBJ) -o bin/Power_Hypervisor_test $(LDFLAGS)

# ===== RUN COMMANDS =====
run-ca: bin/CA_test_port
	./bin/CA_test_port $(CA_PORT)

run-power: bin/Power_Hypervisor_test
	./bin/Power_Hypervisor_test $(POWER_PORT)

run-device1: bin/Device_1_test
	./bin/Device_1_test 127.0.0.1 $(CA_PORT) 127.0.0.1 $(POWER_PORT)

run-device2: bin/Device_2_test
	./bin/Device_2_test 127.0.0.1 $(CA_PORT) 127.0.0.1 $(POWER_PORT)

run-device3: bin/Device_3_test
	./bin/Device_3_test 127.0.0.1 $(CA_PORT) 127.0.0.1 $(POWER_PORT)