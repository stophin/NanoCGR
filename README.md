# NanoCGR  
NanoCGR: CGR Server  
  
What's Included:  
  Unicode to ANSI/UTF-8 encoding;
  HTTP listen and response;
  WebSocket handshack, frame encode/decocde and response;
  HTTPS listen and response(not completed);
  
  
HOWTO:  
Linux:  
1.make or make d: make dynamic lib  
  make s: make ststic lib  
  NOTE: There are some problems making static lib, which will lead to  
    instance loading failure. Using dynamic lib is suggested.  
  That's all done.   
  Use make run_d or make run_s to run with the lib you built(s: ststic, d: dynamic).  
  
Windows: using vs2013 to build the project.  
  
