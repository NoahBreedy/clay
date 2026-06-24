CC := gcc

# -----------------------------
# EDIT ME WITH RELATIVE PATH TO:
#
# tigr.c & tigr.h
# -----------------------------
TIGR_DIR := <PATH_TO_TIGR>

OUTFILE := demo

CFLAGS :=
LDFLAGS := -lm

# -----------------------------
# platform flags
# -----------------------------
ifeq ($(OS),Windows_NT)
    LDFLAGS += -lopengl32 -lgdi32
    RM := del /q
else
    OS := $(shell uname -s)

    ifeq ($(OS),Darwin)
        LDFLAGS += -framework OpenGL -framework Cocoa
    else ifeq ($(OS),Linux)
        LDFLAGS += -lGLU -lGL -lX11
    endif

    RM := rm -f
endif

SRCS := \
	main.c \
    $(TIGR_DIR)/tigr.c \
    ../../renderers/tigr/tigr_utils.c \
    ../../renderers/tigr/clay_renderer_tigr.c

all: main.c
	$(CC) -I$(TIGR_DIR) $(CFLAGS) $(SRCS) -o $(OUTFILE) $(LDFLAGS)

clean:
	$(RM) $(OUTFILE)
