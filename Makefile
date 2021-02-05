apps_src += $(addprefix apps/terminal/,\
  terminal.cpp \
  software/kilo.cpp \
  list.cpp \
  external/archive.cpp \
  external/extapp_api.cpp \
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