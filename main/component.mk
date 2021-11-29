#
# Main Makefile. This is basically the same as a component makefile.
#

GIT_PROJECT_VER := $(shell cd ${PROJECT_PATH} && git describe --always --tags --dirty 2> /dev/null)

http.o: CFLAGS += -DVERSION=\""$(GIT_PROJECT_VER)"\"
