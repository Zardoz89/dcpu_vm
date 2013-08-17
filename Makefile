.PHONY: clean All

All:
	@echo "----------Building project:[ DCPU - Debug ]----------"
	@$(MAKE) -f  "DCPU.mk"
clean:
	@echo "----------Cleaning project:[ DCPU - Debug ]----------"
	@$(MAKE) -f  "DCPU.mk" clean
