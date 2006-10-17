// ---------------------------------------------------------------------------
// NAME               : Perfmeter3.java
// VERSION            : 3.0.1
// AUTHOR             : Marko Koscak
// LAST REVISION DATE : Jun 06 2001
// NOTES              :
// ---------------------------------------------------------------------------

// -----------------------
// Import java class libs.
// -----------------------
import java.awt.*;
import java.awt.event.*;
import java.lang.*;

import java.net.Socket;
import java.io.IOException;
import java.io.DataInputStream;
import java.io.DataOutputStream;


// ------------------
// Define main class.
// ------------------
public class Perfmeter3 extends Frame implements WindowListener
{

public class Perfcanvas extends Canvas implements Runnable
{
	Thread countThread;

	public int	sleepcounter;
	public long router_load;
	public long router_anzahl; // anzahl angekommener Packete
	private int Durchsatz[] = new int[_FIELDLENGTH * 2];  // 285*2 Durchsatzdaten min=0 max=110
	private int Position;
		
	private boolean program_exit;

	public void start() {
		sleepcounter = 100;
		Position = 0;
		router_load = 0;
		router_anzahl = 0;
		program_exit = false;
		
		for( int i = 0; i < (_FIELDLENGTH * 2); i++ ) {
			Durchsatz[i] = _FIELDHEIGHT;
		}
		repaint();

		// setup Thread
		if( countThread == null ) {
			countThread = new Thread( this );
			countThread.setPriority( Thread.NORM_PRIORITY );
//			countThread.setPriority( Thread.MAX_PRIORITY );	// for realtime display rendering
			countThread.start();
		}
	}

	public void stop() {
		program_exit = true;
	}

	public void paint( Graphics g )
	{
	    g.setColor( getBackground() );
	    g.draw3DRect( 1, 1, _FIELDLENGTH + 1, _FIELDHEIGHT + 1, false );
	    g.setColor( new Color( 0, 0, 0 ) );
		
	    for(int i = _FIELDLENGTH - 1; i >= 2; i-- ) {
			g.drawLine( i, _FIELDHEIGHT, i, Durchsatz[i + Position] );
	    }
	}

	public void run() {
	    long	router_data;
	    long	router_count;
	    double	router_perf;
	
	    // loop until we have finished
	    while( true ) {
			
			router_data = router_load;
			router_load = 0;
			router_count = router_anzahl;
			router_anzahl = 0;
			
//			if( router_count != 0 ) System.out.println( "Anzahl Packete: " + router_count ); 		
			// arithmetischen Mittelwert berechnen
			if( router_count != 0 ) router_data = router_data / router_count;

			// durchsatz berechnen
			router_perf =  110 - (110 * router_data/(10000000));

				
			if( Position == _FIELDLENGTH ) Position = 0;
		
			Durchsatz[Position] = (int)router_perf;
			Durchsatz[_FIELDLENGTH + Position ] = (int)router_perf;
			Position += 1;
	
			repaint();

			try {
				Thread.currentThread().sleep( sleepcounter );
			} catch( InterruptedException e ) { System.out.println("Exeption on sleep");}
	      	}
	}

}

// Class for the threaded client connection
class Socketthread extends Thread
{
	public void run() {
		boolean error = false;

		while( Perfview.program_exit != true ) {
	
			Sockthread_running = true;		
			Socket sock = null;
			try {
			    Integer IntegerPort = new Integer( 1 );
			    sock = new Socket( IPAddress.getText(), IntegerPort.parseInt( Port.getText() ));

			    DataInputStream server_in = new DataInputStream( sock.getInputStream() );
			    DataOutputStream server_out = new DataOutputStream( sock.getOutputStream() );

			    int zeichen1 = 0;
				double router_load;
	    	
				Status_txt.setText( "Status: Connected to Server at Port: " + Port.getText() );
	    	
				while (true) {
					zeichen1 = server_in.readInt();
					if ( zeichen1 == -1 ) {
						// Server quits...
					    break;	
					}
                                        System.out.println("habe empfangen:"+zeichen1);
					Perfview.router_load += zeichen1; // the most important line :-)
					Perfview.router_anzahl += 1;
				

					if( Perfview.program_exit == true ) {
						System.out.println( "Sending abort to Server...\n" );
						Status_txt.setText( "Status: Sending abort to Server..." );
						server_out.writeByte( 'a' );		// tell the server that we will quit
								
						// exit program
						Perfview.countThread.stop();
						Perfview.countThread = null;
								
						System.exit(0);
					}
					else
					{
						server_out.writeByte( 'c' );	// "Verbindung nicht beenden" signalisieren
					                                // (c - continue, a - abort )
	    			}
	    		}
			}
			catch ( IOException e )
			{
//				System.out.println( "An Error is occured (IOException): " + e + "\n" );
				Status_txt.setText( "Status: " + e.getLocalizedMessage() );
				error = true;
			}
			catch ( NumberFormatException e1 )
			{
				System.out.println( "An Error is occured (NumberFormatException): " + e1 + "\n" );
				Status_txt.setText( "Status: " + e1.getLocalizedMessage() );	
				error = true;
			}
			
			finally
			{
		    	try {
					if (sock != null) sock.close();
	    		}
		    	catch ( IOException e2 ) {
		    		System.out.println( "An Error is occured (IOException): " + e2 + "\n" );
					Status_txt.setText( "Status: " + e2.getLocalizedMessage() );
					error = true;
				}
			}
			
			if( !error ) Status_txt.setText( "Status: Disconnected" );
		
			Sockthread_running = false;

			try {
				Thread.currentThread().sleep(500);
			} catch( InterruptedException e ) {
				System.out.println("Exeption on sleep");
			}
		}
	}
}


	public final static int _FIELDLENGTH = 234;
	public final static int _FIELDHEIGHT = 110;

	public final static int _MAXANZEIGE  = 10; // in MB/s
	
	

	// --------------------------------
	// Define private member variables.
	// --------------------------------
	private Label IPAddress_txt = null;
	private TextField IPAddress = null;
	private Label doppelpkt = null;
	private TextField Port = null;
	private Label Port_txt = null;
	private Button Startbutton = null;
	private Label Anzeige100_txt = null;
	private Label Anzeige50_txt = null;
	private Label Anzeige0_txt = null;
   	public  Label Status_txt = null;
	private Label Speed_txt = null;
	
	private Perfcanvas Perfview = null;
   	
   	Socketthread Sockthread;
   	boolean Sockthread_running;
	boolean speed;

    // -----------------------------
    // Define constructor for class.
    // -----------------------------
    public Perfmeter3()
    {

        // Call super constructor.
        super( "Perfmeter Application" );

        // Set up container component.
        setBackground( new Color( 192, 192, 192 ) );
        setForeground( new Color( 0, 0, 0 ) );
        setSize( new Dimension( 300, 225 ) );
        setLocation( 100, 100 );
//		setBounds( 100, 75, 640, 300 );
        setResizable( false );

        // Add listeners.
        addWindowListener( this );

        // Thread for the comminication with the server
        Sockthread_running = false;
		speed = true;
		
        // Make the GUI.
        makeGUI();
		Perfview.start();	// start Thread

		Sockthread = new Socketthread();
		Sockthread.setPriority( Thread.NORM_PRIORITY );
		Sockthread.start();
	
		show();
    }

    // ---------------------------
    // Define constructor for GUI.
    // ---------------------------
    private void makeGUI()
    {

        // Create the GUI system.
        this.setLayout( null );
        IPAddress_txt = new Label( "IP Address:", Label.LEFT );
        IPAddress_txt.setBounds( new Rectangle( new Dimension( 130, 23 ) ) ); 
        IPAddress_txt.setLocation( 6, 23 );
        add( IPAddress_txt );
        IPAddress_txt.setBackground( new Color( 192, 192, 192 ) );
        IPAddress_txt.setForeground( new Color( 0, 0, 0 ) );
        IPAddress_txt.setEnabled( true );
        IPAddress_txt.setVisible( true );
        IPAddress = new TextField( "134.130.62.103", 20 );
        IPAddress.setBounds( new Rectangle( new Dimension( 130, 30 ) ) ); 
        IPAddress.setLocation( 6, 46 );
        add( IPAddress );
        IPAddress.setBackground( new Color( 192, 192, 192 ) );
        IPAddress.setForeground( new Color( 0, 0, 0 ) );
        IPAddress.setEnabled( true );
        IPAddress.setVisible( true );
        doppelpkt = new Label( ":", Label.CENTER );
        doppelpkt.setBounds( new Rectangle( new Dimension( 14, 15 ) ) ); 
        doppelpkt.setLocation( 136, 53 );
        add( doppelpkt );
        doppelpkt.setBackground( new Color( 192, 192, 192 ) );
        doppelpkt.setForeground( new Color( 0, 0, 0 ) );
        doppelpkt.setEnabled( true );
        doppelpkt.setVisible( true );
        Port = new TextField( "5000", 20 );
        Port.setBounds( new Rectangle( new Dimension( 62, 30 ) ) ); 
        Port.setLocation( 150, 46 );
        add( Port );
        Port.setBackground( new Color( 192, 192, 192 ) );
        Port.setForeground( new Color( 0, 0, 0 ) );
        Port.setEnabled( true );
        Port.setVisible( true );
        Port_txt = new Label( "Port:", Label.LEFT );
        Port_txt.setBounds( new Rectangle( new Dimension( 62, 23 ) ) ); 
        Port_txt.setLocation( 150, 23 );
        add( Port_txt );
        Port_txt.setBackground( new Color( 192, 192, 192 ) );
        Port_txt.setForeground( new Color( 0, 0, 0 ) );
        Port_txt.setEnabled( true );
        Port_txt.setVisible( true );
        Speed_txt = new Label( "Speed:", Label.LEFT );
        Speed_txt.setBounds( new Rectangle( new Dimension( 62, 23 ) ) ); 
        Speed_txt.setLocation( 233, 23 );
        add( Speed_txt );
        Speed_txt.setBackground( new Color( 192, 192, 192 ) );
        Speed_txt.setForeground( new Color( 0, 0, 0 ) );
        Speed_txt.setEnabled( true );
        Speed_txt.setVisible( true );
        Startbutton = new Button( "fast" );
        Startbutton.setBounds( new Rectangle( new Dimension( 38, 24 ) ) ); 
        Startbutton.setLocation( 235, 48 );
        add( Startbutton );
        Startbutton.setBackground( new Color( 192, 192, 192 ) );
        Startbutton.setForeground( new Color( 0, 0, 0 ) );
        Startbutton.setEnabled( true );
        Startbutton.setVisible( true );
        Perfview = new Perfcanvas();
        Perfview.setBounds( new Rectangle( new Dimension( _FIELDLENGTH + 4, _FIELDHEIGHT + 4 ) ) ); 
        Perfview.setLocation( 5, 80 );
        add( Perfview );
        Perfview.setBackground( new Color( 192, 192, 192 ) );
        Perfview.setForeground( new Color( 0, 0, 0 ) );
        Perfview.setEnabled( true );
        Perfview.setVisible( true );

        Anzeige100_txt = new Label( _MAXANZEIGE + " MB/s", Label.LEFT );
        Anzeige100_txt.setBounds( new Rectangle( new Dimension( 62, 23 ) ) );
        Anzeige100_txt.setLocation( 240, 75);
        add( Anzeige100_txt );
        Anzeige100_txt.setBackground( new Color( 192, 192, 192 ) );
        Anzeige100_txt.setForeground( new Color( 0, 0, 0 ) );
        Anzeige100_txt.setEnabled( true );
        Anzeige100_txt.setVisible( true );
        Anzeige50_txt = new Label( _MAXANZEIGE/2 + " MB/s", Label.LEFT );
        Anzeige50_txt.setBounds( new Rectangle( new Dimension( 62, 23 ) ) );
        Anzeige50_txt.setLocation( 248, 125 );
        add( Anzeige50_txt );
        Anzeige50_txt.setBackground( new Color( 192, 192, 192 ) );
        Anzeige50_txt.setForeground( new Color( 0, 0, 0 ) );
        Anzeige50_txt.setEnabled( true );
        Anzeige50_txt.setVisible( true );
        Anzeige0_txt = new Label( "0 MB/s", Label.LEFT );
        Anzeige0_txt.setBounds( new Rectangle( new Dimension( 62, 23 ) ) );
        Anzeige0_txt.setLocation( 248, 175 );
        add( Anzeige0_txt );
        Anzeige0_txt.setBackground( new Color( 192, 192, 192 ) );
        Anzeige0_txt.setForeground( new Color( 0, 0, 0 ) );
        Anzeige0_txt.setEnabled( true );
        Anzeige0_txt.setVisible( true );

        Status_txt = new Label( "Status: Disconnected",Label.LEFT );
        Status_txt.setBounds( new Rectangle( new Dimension( 280, 23 ) ) );
        Status_txt.setLocation( 6, 198 );
        add( Status_txt );
    }

    // --------------------------------------------
    // Define a 'make grid bag constraints' method.
    // --------------------------------------------
    public GridBagConstraints makeGridCons( int x, int y, int dx, int dy, int wx, int wy, int fill, int anch, int it, int il, int ib, int ir, int px, int py )
    {

        // Define local variables.
        GridBagConstraints cons = new GridBagConstraints();

        // Set the constraints.
        cons.weightx    = wx;
        cons.weighty    = wy;
        cons.gridx      = x;
        cons.gridy      = y;
        cons.gridwidth  = dx;
        cons.gridheight = dy;
        cons.fill       = fill;
        cons.anchor     = anch;
        cons.insets     = new Insets( it, il, ib, ir );
        cons.ipadx      = px;
        cons.ipady      = py;

        // Return the constraints to the caller.
        return cons;

    }

    public boolean action( Event evt, Object whatAction ) {

		if( !speed ) {
			Perfview.sleepcounter = 100;
			speed = true;
			Startbutton.setLabel( "fast" );
		}
		else {
			Perfview.sleepcounter = 1000;
			speed = false;
			Startbutton.setLabel( "slow" );
		} 
		return true;
    }


    // ------------------------
    // Define window listeners.
    // ------------------------
    public void windowClosing( WindowEvent event )
    {
	// here we must tell the server that we want to exit the connection, so that
	// it will not come into trouble
	// this is done within the Socketthread
	Perfview.stop();
        dispose();
    }
    public void windowOpened( WindowEvent event ) {}
    public void windowIconified( WindowEvent event ) {}
    public void windowDeiconified( WindowEvent event ) {}
    public void windowClosed( WindowEvent event ) {}
    public void windowActivated( WindowEvent event ) {}
    public void windowDeactivated( WindowEvent event ) {}

    public static void main( String[] args )
    {
    	new Perfmeter3();
    }
}
