gvm_MODULES:= main cpu ram converter symbol opcode
ifeq ($(PLATFORM),win32)
gvm_MODULES+= builtin_getopt
gvm_DEFINES+= -DBUILTIN_GETOPT
endif
$(call binrules, gvm)

