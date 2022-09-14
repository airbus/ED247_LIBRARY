# Configure a file
# Variables:
# - INFILE
# - OUTFILE
# - all variables you want to configure

configure_file(${INFILE} ${OUTFILE} @ONLY)

# configure_file do not update date/time if output content do not change
file(TOUCH_NOCREATE ${OUTFILE})
