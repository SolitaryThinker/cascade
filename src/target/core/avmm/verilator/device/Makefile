all:
	verilator -O3 --cc -Wno-lint -Wno-fatal program_logic.v --exe sim_main.cpp
	make -k -C obj_dir -f Vprogram_logic.mk Vprogram_logic

prepare:
	sed -i '1i // verilator lint_off BLKANDNBLK' program_logic.v

clean:
	rm -rf obj_dir/
