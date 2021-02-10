apps_src += $(addprefix apps/terminal/,\
  terminal.cpp \
  software/kilo.cpp \
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