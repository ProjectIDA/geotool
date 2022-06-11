INCLUDES= -I$(top_srcdir)/include -I$(top_srcdir)/include/motif++ $(INTERACTIVE_IPC_INCLUDES)

LIBS = 

lib_LTLIBRARIES = libmotif++.la

if HAVE_INTERACTIVE_IPC
libmotif___la_LIBADD= \
	$(INTERACTIVE_IPC_LIBS)
endif



libmotif___la_SOURCES = \
		Application.cpp \
		AppParse.cpp \
		ArrowButton.cpp \
		Button.cpp \
		Choice.cpp \
		Component.cpp \
		FileChoice.cpp \
		FileDialog.cpp \
		FormDialog.cpp \
		Frame.cpp \
		Info.c \
		InfoArea.cpp \
		IPCClient.cpp \
		List.cpp \
		ListDialog.cpp \
		MenuBar.cpp \
		Menu.cpp \
		ParamDialog.cpp \
		Parse.cpp \
		PlugInManager.cpp \
		Question.cpp \
		RowColumn.cpp \
		Scale.cpp \
		ScrollBar.cpp \
		ScrolledPane.cpp \
		StatusDialog.cpp \
		TextDialog.cpp \
		TextField.cpp \
		TextQuestion.cpp \
		Toggle.cpp \
		ToolBar.cpp \
		TopWindow.cpp \
		UndoButton.cpp \
		UndoManager.cpp \
		Warn.cpp \
		Warnings.cpp

