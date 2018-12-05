SourceDirectory: /path/to/project/source/directory
BuildDirectory: /path/to/project/build/directory
CVSUpdateOptions: -d -A -P
Site: my-system-name
BuildName: build-name
DropSite: public.kitware.com
DropLocation: /cgi-bin/HTTPUploadDartFile.cgi
DropMethod: http
TriggerSite: http://public.kitware.com/cgi-bin/Submit-Random-TestingResults.pl
NightlyStartTime: 21:00:00 EDT
ConfigureCommand: "/path/to/source/directory/Project/configure"
MakeCommand: /usr/bin/make -i
CVSCommand: /usr/bin/cvs
CoverageCommand: /usr/bin/gcov
TimeOut: 1500
