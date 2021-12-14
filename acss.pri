
CONFIG(debug, debug|release){
    win32 {
    	versionAtLeast(QT_VERSION, 5.15.0) {
    		LIBS += -lqtadvancedcss
    	}
    	else {
    		LIBS += -lqtadvancedcssd
    	}
    }
    else:mac {
        LIBS += -lqtadvancedcss_debug
    }
    else {
        LIBS += -lqtadvancedcss
    }
}
else{
    LIBS += -lqtadvancedcss
}
