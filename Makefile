ifeq ($(word 1, $(MAKECMDGOALS)),scene)
	TARGET := $(word 2, $(MAKECMDGOALS))
endif

CC := g++
CFLAGS := -std=c++1y

DIRS := \
	common   \
	input    \
	render   \
	resource

LIBS := \
	-framework OpenGL \
	-lglfw3           \
	-lglew

HEADER := $(wildcard src/*/*.h)
SOURCE := $(wildcard src/*/*.cpp)
SOURCE := $(filter-out src/scene/%.cpp, $(SOURCE))
SOURCE += src/scene/$(TARGET).cpp

OBJECT := bin/obj/main.o
OBJECT += $(subst src/,bin/obj/, $(addsuffix .o, $(basename $(SOURCE))))
DEPEND := $(subst obj/,dep/, $(addsuffix .d, $(basename $(OBJECT))))

DIR := $(sort $(dir $(OBJECT))) $(sort $(dir $(DEPEND)))
BINARY := main

.PHONY: scene $(TARGET)

$(TARGET): $(DEPEND) bin/$(BINARY)
scene:
	@true

check: $(DIR)

bin/dep/%.d: src/%.cpp | check
	$(CC) $(CFLAGS) -MM -MP -MT bin/obj/$*.o $< > $@
bin/obj/%.o: src/%.cpp | check
	$(CC) $(CFLAGS) -c -o $@ $<

bin/$(BINARY): bin/obj/main.o $(OBJECT)
	$(CC) -o bin/$(BINARY) $^ $(LIBS)

$(DIR):
	@mkdir -p $@

clean:
	@rm -rf bin/*

-include $(DEPEND)
