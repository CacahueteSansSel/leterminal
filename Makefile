apps_src += $(addprefix apps/terminal/,\
  system/users.cpp \
  software/kilo.cpp \
  vfs/vfs.cpp \
  strings.cpp \
  stringx.cpp \
  terminal.cpp \
  list.cpp \
)

ifeq ($(FIRMWARE),sigma)
	SFLAGS += -DSIGMA
else

ifeq ($(FIRMWARE),omega)
	SFLAGS += -DOMEGA
else
  SFLAGS += -DEPSILON
endif

endif