TESTER_C = tester_src/tester_main.c

ASSIGNMENTS ?= 1_0 1_1 1_2 1_3 1_4 1_5 1_6 1_7 2_0 2_1 3_0 3_1 3_2 3_3 4_0 4_1 5_0
ASSIGNMENTS_S ?= $(addsuffix .s, $(addprefix assignments/, $(ASSIGNMENTS)))
ASSIGNMENTS_O ?= $(addsuffix .o, $(addprefix build/assignments/, $(ASSIGNMENTS)))

APP = build/app
PK = /nix/store/1yyy5ragpgw1kg4dyc4dp3sjzvwvsm8r-riscv-pk/bin/pk

.PHONY: grade
grade: build/grade.txt
	@cat $<

build/grade.txt: build/exec_out.txt $(ASSIGNMENTS_S)
	@python3 scripts/check_success_and_objects.py $^ $@

.PHONY: details
details: $(APP)
	@spike --isa=RV64IMAFDC $(PK) $<

.PHONY: zip
zip:
	tar zcf handout.tar.gz assignments/*.s

build/exec_out.txt: $(APP) | build
	@spike --isa=RV64IMAFDC $(PK) $< > $@

$(APP): $(TESTER_C) $(ASSIGNMENTS_O)
	@riscv64-unknown-elf-gcc -O $^ -o $@

build/%.o: %.s | build/assignments
	@riscv64-unknown-elf-gcc -O -c $^ -o $@

$(TESTER_O): $(TESTER_C)
	@riscv64-unknown-elf-gcc -O -c $< -o $@

build build/assignments:
	@mkdir -p $@

.PHONY: clean
clean:
	rm -rf build
