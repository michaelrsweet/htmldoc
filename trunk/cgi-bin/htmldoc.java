//
// "$Id: htmldoc.java,v 1.2 2004/03/31 07:01:55 mike Exp $"
//
//   Java interface to HTMLDOC.
//
//   Copyright 2001 by Easy Software Products.
//
//   These coded instructions, statements, and computer programs are the
//   property of Easy Software Products and are protected by Federal
//   copyright law.  Distribution and use rights are outlined in the file
//   "COPYING.txt" which should have been included with this file.  If this
//   file is missing or damaged please contact Easy Software Products
//   at:
//
//       Attn: ESP Licensing Information
//       Easy Software Products
//       44141 Airport View Drive, Suite 204
//       Hollywood, Maryland 20636-3111 USA
//
//       Voice: (301) 373-9600
//       EMail: info@easysw.com
//         WWW: http://www.easysw.com
//


class htmldoc
{
  // Convert named file to PDF on stdout...
  public static int topdf(String filename)// I - Name of file to convert
  {
    String		command;	// Command string
    Process		process;	// Process for HTMLDOC
    Runtime		runtime;	// Local runtime object
    java.io.InputStream	input;		// Output from HTMLDOC
    byte		buffer [];	// Buffer for output data
    int			bytes;		// Number of bytes


    // First tell the client that we will be sending PDF...
    System.out.print("Content-type: application/pdf\n\n");

    // Construct the command string
    command = "htmldoc --quiet --jpeg --webpage -t pdf --left 36 " +
              "--header .t. --footer .1. " + filename;

    // Run the process and wait for it to complete...
    runtime = Runtime.getRuntime();

    try
    {
      // Create a new HTMLDOC process...
      process = runtime.exec(command);

      // Get stdout from the process and a buffer for the data...
      input  = process.getInputStream();
      buffer = new byte[8192];

      // Read output from HTMLDOC until we have it all...
      while ((bytes = input.read(buffer)) > 0)
        System.out.write(buffer, 0, bytes);

      // Return the exit status from HTMLDOC...
      return (process.waitFor());
    }
    catch (Exception e)
    {
      // An error occurred - send it to stderr for the web server...
      System.err.print(e.toString() + " caught while running:\n\n");
      System.err.print("    " + command + "\n");
      return (1);
    }
  }

  // Main entry for htmldoc class
  public static void main(String[] args)// I - Command-line args
  {
    String	server_name,		// SERVER_NAME env var
		server_port,		// SERVER_PORT env var
		path_info,		// PATH_INFO env var
		query_string,		// QUERY_STRING env var
		filename;		// File to convert


    if ((server_name = System.getProperty("SERVER_NAME")) != null &&
        (server_port = System.getProperty("SERVER_PORT")) != null &&
	(path_info = System.getProperty("PATH_INFO")) != null)
    {
      // Construct a URL for the resource specified...
      filename = "http://" + server_name + ":" + server_port + path_info;

      if ((query_string = System.getProperty("QUERY_STRING")) != null)
      {
        filename = filename + "?" + query_string;
      }
    }
    else if (args.length == 1)
    {
      // Pull the filename from the command-line...
      filename = args[0];
    }
    else
    {
      // Error - no args or env variables!
      System.err.print("Usage: htmldoc.class filename\n");
      return;
    }

    // Convert the file to PDF and send to the web client...
    topdf(filename);
  }
}


//
// End of "$Id: htmldoc.java,v 1.2 2004/03/31 07:01:55 mike Exp $".
//
