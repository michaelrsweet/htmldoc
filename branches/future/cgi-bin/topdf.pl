sub topdf(filename);

sub topdf {
    print "Content-Type: application/pdf\n\n"
    system htmldoc -t pdf --webpage $filename
}
