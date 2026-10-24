# Host-Tests der Firmware-Kernlogik: `make` baut und startet die Tests.
CXX      ?= g++
SRC_DIR  := ../../ShishaLed
CXXFLAGS := -std=c++17 -O1 -g -Wall -Wextra -Werror -I$(SRC_DIR) -fsanitize=address,undefined -fno-sanitize-recover=all

CORE  := color_math utf8_util config_manager pull_detector session_manager led_logic
TESTS := test_main test_color test_utf8 test_config test_sensor test_session test_led

SRCS := $(addprefix $(SRC_DIR)/,$(addsuffix .cpp,$(CORE))) $(addsuffix .cpp,$(TESTS))

run: host_tests
	./host_tests

host_tests: $(SRCS) $(wildcard $(SRC_DIR)/*.h) $(wildcard *.h)
	$(CXX) $(CXXFLAGS) -o $@ $(SRCS)

clean:
	rm -f host_tests

.PHONY: run clean
