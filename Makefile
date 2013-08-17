.PHONY: clean All

All:
	@echo "----------Building project:[ DCPU - Release ]----------"
	@$(MAKE) -f  "DCPU.mk"
clean:
	@echo "----------Cleaning project:[ DCPU - Release ]----------"
	@$(MAKE) -f  "DCPU.mk" clean
