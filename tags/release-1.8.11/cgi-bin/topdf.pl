sub topdf(filename);

sub topdf {
    my $filename = shift;

    print "Content-Type: application/pdf\n\n";
    system "htmldoc -t pdf --webpage $filename";
}
