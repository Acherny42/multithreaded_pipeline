# multithreaded_pipeline
This project consists of three threads interconnected with links.
First, is a generator that generates protocol packets.
It uses API to fill in the fields that may cross byte borders. 
It sends those packets through a link to a detector that extracts values from the packet and validates those packets according to some predefined criterion.
The status of each received packet is then sent to the logger, which writes those entries into the log file. 
For development purposes, all three may send their debug logs to the debug logger through another link, to get ordered logfile.
Each thread may be started and stopped.
Each link may be detached to signal a sleeping thread on another end.
