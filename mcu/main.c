/**
 * @name Simulation of musical instrument on MCU MSP430F168 (Texas Instruments)
 * @file main.c 
 * @author Lukas Tkac (login: xtkacl00) <xtkacl00 AT stud.fit.vutbr.cz>
 */

#include <fitkitlib.h>
#include <keyboard/keyboard.h>
#include <lcd/display.h>

/**
 * DEFINICIA FREKVENCII TONOV PRE 2 OKTAVY
 * Jednociarkova oktava c' d' e' f' g' a' h'
 * Dvojciarkova oktava c'' d'' e'' f'' g'' a'' h''
 *
 * Jednotlive frekvencie boli cerpane z tychto zdrojov:
 * 1) https://pages.mtu.edu/~suits/notefreqs.html
 * 2) http://radkon.eu/projects/other/tones.php?lang=sk
 * 
 * !!! POZOR 0 !!! vsetky frekvencie su nadefinovane v Hertzoch [Hz]
 * !!! POZOR 1 !!! v definiciach su pomenovane jednotlive tony medzinarodnou notaciou (napr. c' v medzinarodnej notacii je C4)
 * !!! POZOR 2 !!! nie je to "perfect tone" frekvencie zaokruhlene na cele cislo - desatinna cast zaokruhlena nahor
 * (napr 12.4Hz bude 13Hz)
 *
 */

// definice kontrolnych stavov funkcii
#define PROCESS_OK 0
#define PROCESS_ERR 1

// jednociarkova oktava
#define C4 262 // c'
#define D4 294 // d'
#define E4 330 // e' 
#define F4 350 // f'
#define G4 392 // g'
#define A4 440 // a'
#define B4 494 // h'

//dvojciarkova oktava
#define C5 524 // c''
#define D5 588 // d''
#define E5 660 // e''
#define F5 699 // f''
#define G5 784 // g''
#define A5 880 // a''
#define B5 988 // h''


// nezadefinovane noty pre demo  a potrebne pre DEMO skladbu 
#define G3 196 // g0
#define GS3 208 // gis0 /as0
#define E3 165 // e0 
#define A3 220 // a0
#define AS3 234 // 	ais0 / b0
#define B3 247 // b0
#define FS4 370 // mozno Fs4 notaciu som nedohladal  tipujem to na FiS
#define AB4 416 // gis' / as'
#define BB4 467 // ais' / b'
#define EB4 312 // dis'

#define EB5 623 // dis'' / es''



// pre generovanie tonu nastroja pouzijeme obdlznikove vzorky signalu
#define SQUARE_SAMPLES 2  // pocet vzorku obdelnikoveho signalu

// definicia poctu tikov za sekundu
#define TICKS_PER_SECOND 32768 

// definicia tvaru obdlznikoveho signalu
unsigned char arr_square[SQUARE_SAMPLES] = {0, 255};

// globalne premenne pre chod generatoru signalu ( generatoru signalov o frekvnecii )
unsigned int freq = C4;  // defaultna frekvencia nastavena na ton C4
unsigned int amplitude_scale_x = 0; // amplituda napetia na x-ovej osi vyjadrena v %, kde aplitude_scale_x patri do intervalu <0,100> 
unsigned int sample_iterator = 1; // defaultne 1 aby bolo zabezbene kmitanie medzi urovnami v log ponimani(kde swap urovne je korigovany prerusenim): 1 0 1 0 1 0

// Pocet tikov je pocitany na zaklade tohoto matematickeho vztahu ticks = TICKS_PER_SECOND / frequency / samples;
unsigned int ticks;

// prototypy funkcii pre potreby vykonania skor nez main() alebo pouzitia v main()
void part1_demo();
void part2_demo();
void part3_demo();
void part4_demo();
void part5_demo();
void part6_demo();
void part7_demo();
void part8_demo();
void play_demo();
interrupt (TIMERA0_VECTOR) Timer_A (void);
void print_user_help(void);
void fpga_initialized();
unsigned char decode_user_cmd(char *UserCommand, char *ComparedCommand);
char tone_decoder(unsigned int keyboard_input);
int keyboard_idle();


// Hlavna funkcia main pre obsluhu s hlavnym cyklom pre obsluhu klavesnice a terminalu
int main(void)
{
    initialize_hardware();
    WDG_stop(); //stop watchdog char_cnt
    
    // inicializacia tikov na zaklade podielu tikov za sekundu, frequencie daneho tonu a poctom vzoriek signalu (v nasom pripade obdlznikovy)
    ticks = TICKS_PER_SECOND/freq/SQUARE_SAMPLES;

    // Nastavenie casovaca (pouziti demo kod s blikajucou LED)
    CCTL0 = CCIE; // povolenie prerusenia pre casovac (rezim vstupnej komparacie)
    CCR0 = ticks; // pocet tikov, po ktorych pride k preruseniu
    TACTL = TASSEL_1 + MC_2; // ACLK (f_tiku = 32768 Hz = 0x8000 Hz), nepretrzity rezim

    
    /**
     * Potrebujem na analogovu periferiu (reproduktor) prekonvertovat digitalne zlozky, takze si to vyzaduje aby bol pouzity DA prevodnik
     * Nastavenie AD prevodniku, v DA prevodniku sa odkazujeme na referencne napetie, 
     * vychadzam z dokumentacie a nadobudol som predpokladu ze si ho musim nastavit najprv
     */ 
    
    ADC12CTL0 |= 0x0020;    // nastavenie refeencneho napetia na 1,5 V, je mozne ist az na 2,5V.
    DAC12_0CTL |= 0x1060;   // nastavenie kontrolneho registra DAC (na 8-bitovy rezim, medium speed)
    DAC12_0CTL |= 0x100;    // referencne napeti nasobit 1x, podla dokumentacie je mozne nasobit referencne napetie aj 3x, co myslim ze tu nepotrebujem

    while (1)
    {   
        keyboard_idle();
        terminal_idle();
    }
}

    void part1_demo() {

        freq = E4;
        ticks = TICKS_PER_SECOND/freq/SQUARE_SAMPLES;
		amplitude_scale_x = 100; // zapnutie hlasitosti a zahratie dekodovaneho tonu z klavesnice		
		delay_ms(150);
		amplitude_scale_x = 0;
        
        freq = E4;
        ticks = TICKS_PER_SECOND/freq/SQUARE_SAMPLES;
		amplitude_scale_x = 100; // zapnutie hlasitosti a zahratie dekodovaneho tonu z klavesnice		
		delay_ms(150);
		amplitude_scale_x = 0;

        delay_ms(150);

        freq = E4;
        ticks = TICKS_PER_SECOND/freq/SQUARE_SAMPLES;
		amplitude_scale_x = 100; // zapnutie hlasitosti a zahratie dekodovaneho tonu z klavesnice		
		delay_ms(150);
		amplitude_scale_x = 0;

        freq = C4;
        ticks = TICKS_PER_SECOND/freq/SQUARE_SAMPLES;
		amplitude_scale_x = 100; // zapnutie hlasitosti a zahratie dekodovaneho tonu z klavesnice		
		delay_ms(150);
		amplitude_scale_x = 0;

        freq = E4;
        ticks = TICKS_PER_SECOND/freq/SQUARE_SAMPLES;
		amplitude_scale_x = 100; // zapnutie hlasitosti a zahratie dekodovaneho tonu z klavesnice		
		delay_ms(150);
		amplitude_scale_x = 0;
    
        delay_ms(150);

        freq = G4;
        ticks = TICKS_PER_SECOND/freq/SQUARE_SAMPLES;
		amplitude_scale_x = 100; // zapnutie hlasitosti a zahratie dekodovaneho tonu z klavesnice		
		delay_ms(150);
		amplitude_scale_x = 0;
    
        delay_ms(450);
    
        freq = G3;
        ticks = TICKS_PER_SECOND/freq/SQUARE_SAMPLES;
		amplitude_scale_x = 100; // zapnutie hlasitosti a zahratie dekodovaneho tonu z klavesnice		
		delay_ms(150);
		amplitude_scale_x = 0;

        delay_ms(450);
    }
    
    void part2_demo() {
        freq = C4;
        ticks = TICKS_PER_SECOND/freq/SQUARE_SAMPLES;
		amplitude_scale_x = 100; // zapnutie hlasitosti a zahratie dekodovaneho tonu z klavesnice		
		delay_ms(150);
		amplitude_scale_x = 0;
        
        delay_ms(300);

        freq = G3;
        ticks = TICKS_PER_SECOND/freq/SQUARE_SAMPLES;
		amplitude_scale_x = 100; // zapnutie hlasitosti a zahratie dekodovaneho tonu z klavesnice		
		delay_ms(150);
		amplitude_scale_x = 0;

        delay_ms(300);

        freq = E3;
        ticks = TICKS_PER_SECOND/freq/SQUARE_SAMPLES;
		amplitude_scale_x = 100; // zapnutie hlasitosti a zahratie dekodovaneho tonu z klavesnice		
		delay_ms(150);
		amplitude_scale_x = 0;

        delay_ms(300);

        freq = A3;
        ticks = TICKS_PER_SECOND/freq/SQUARE_SAMPLES;
		amplitude_scale_x = 100; // zapnutie hlasitosti a zahratie dekodovaneho tonu z klavesnice		
		delay_ms(150);
		amplitude_scale_x = 0;

        delay_ms(150);

        freq = B3;
        ticks = TICKS_PER_SECOND/freq/SQUARE_SAMPLES;
		amplitude_scale_x = 100; // zapnutie hlasitosti a zahratie dekodovaneho tonu z klavesnice		
		delay_ms(150);
		amplitude_scale_x = 0;
    
        delay_ms(150);

        freq = AS3;
        ticks = TICKS_PER_SECOND/freq/SQUARE_SAMPLES;
		amplitude_scale_x = 100; // zapnutie hlasitosti a zahratie dekodovaneho tonu z klavesnice		
		delay_ms(150);
		amplitude_scale_x = 0;

        freq = A3;
        ticks = TICKS_PER_SECOND/freq/SQUARE_SAMPLES;
		amplitude_scale_x = 100; // zapnutie hlasitosti a zahratie dekodovaneho tonu z klavesnice		
		delay_ms(150);
		amplitude_scale_x = 0;
        
        delay_ms(150);
    }

    void part3_demo () {
        freq = G3;
        ticks = TICKS_PER_SECOND/freq/SQUARE_SAMPLES;
		amplitude_scale_x = 100; // zapnutie hlasitosti a zahratie dekodovaneho tonu z klavesnice		
		delay_ms(150);
		amplitude_scale_x = 0;

        freq = E4;
        ticks = TICKS_PER_SECOND/freq/SQUARE_SAMPLES;
		amplitude_scale_x = 100; // zapnutie hlasitosti a zahratie dekodovaneho tonu z klavesnice		
		delay_ms(150);
		amplitude_scale_x = 0;

        delay_ms(150);

        freq = G4;
        ticks = TICKS_PER_SECOND/freq/SQUARE_SAMPLES;
		amplitude_scale_x = 100; // zapnutie hlasitosti a zahratie dekodovaneho tonu z klavesnice		
		delay_ms(150);
		amplitude_scale_x = 0;

        freq = A4;
        ticks = TICKS_PER_SECOND/freq/SQUARE_SAMPLES;
		amplitude_scale_x = 100; // zapnutie hlasitosti a zahratie dekodovaneho tonu z klavesnice		
		delay_ms(150);
		amplitude_scale_x = 0;

        delay_ms(150);

        freq = F4;
        ticks = TICKS_PER_SECOND/freq/SQUARE_SAMPLES;
		amplitude_scale_x = 100; // zapnutie hlasitosti a zahratie dekodovaneho tonu z klavesnice		
		delay_ms(150);
		amplitude_scale_x = 0;

        freq = G4;
        ticks = TICKS_PER_SECOND/freq/SQUARE_SAMPLES;
		amplitude_scale_x = 100; // zapnutie hlasitosti a zahratie dekodovaneho tonu z klavesnice		
		delay_ms(150);
		amplitude_scale_x = 0;

        delay_ms(150);

        freq = E4;
        ticks = TICKS_PER_SECOND/freq/SQUARE_SAMPLES;
		amplitude_scale_x = 100; // zapnutie hlasitosti a zahratie dekodovaneho tonu z klavesnice		
		delay_ms(150);
		amplitude_scale_x = 0;

        delay_ms(150);

        freq = C4;
        ticks = TICKS_PER_SECOND/freq/SQUARE_SAMPLES;
		amplitude_scale_x = 100; // zapnutie hlasitosti a zahratie dekodovaneho tonu z klavesnice		
		delay_ms(150);
		amplitude_scale_x = 0;

        freq = D4;
        ticks = TICKS_PER_SECOND/freq/SQUARE_SAMPLES;
		amplitude_scale_x = 100; // zapnutie hlasitosti a zahratie dekodovaneho tonu z klavesnice		
		delay_ms(150);
		amplitude_scale_x = 0;

        freq = B3;
        ticks = TICKS_PER_SECOND/freq/SQUARE_SAMPLES;
		amplitude_scale_x = 100; // zapnutie hlasitosti a zahratie dekodovaneho tonu z klavesnice		
		delay_ms(150);
		amplitude_scale_x = 0;

        delay_ms(300);
    }

    void part4_demo() {
        delay_ms(300);
        
        freq = G4;
        ticks = TICKS_PER_SECOND/freq/SQUARE_SAMPLES;
		amplitude_scale_x = 100; // zapnutie hlasitosti a zahratie dekodovaneho tonu z klavesnice		
		delay_ms(150);
		amplitude_scale_x = 0;

        freq = FS4;
        ticks = TICKS_PER_SECOND/freq/SQUARE_SAMPLES;
		amplitude_scale_x = 100; // zapnutie hlasitosti a zahratie dekodovaneho tonu z klavesnice		
		delay_ms(150);
		amplitude_scale_x = 0;

        freq = F4;
        ticks = TICKS_PER_SECOND/freq/SQUARE_SAMPLES;
		amplitude_scale_x = 100; // zapnutie hlasitosti a zahratie dekodovaneho tonu z klavesnice		
		delay_ms(150);
		amplitude_scale_x = 0;

        freq = EB4;
        ticks = TICKS_PER_SECOND/freq/SQUARE_SAMPLES;
		amplitude_scale_x = 100; // zapnutie hlasitosti a zahratie dekodovaneho tonu z klavesnice		
		delay_ms(150);
		amplitude_scale_x = 0;

        delay_ms(150);

        freq = E4;
        ticks = TICKS_PER_SECOND/freq/SQUARE_SAMPLES;
		amplitude_scale_x = 100; // zapnutie hlasitosti a zahratie dekodovaneho tonu z klavesnice		
		delay_ms(150);
		amplitude_scale_x = 0;

        delay_ms(150);

        freq = GS3;
        ticks = TICKS_PER_SECOND/freq/SQUARE_SAMPLES;
		amplitude_scale_x = 100; // zapnutie hlasitosti a zahratie dekodovaneho tonu z klavesnice		
		delay_ms(150);
		amplitude_scale_x = 0;

        freq = A3;
        ticks = TICKS_PER_SECOND/freq/SQUARE_SAMPLES;
		amplitude_scale_x = 100; // zapnutie hlasitosti a zahratie dekodovaneho tonu z klavesnice		
		delay_ms(150);
		amplitude_scale_x = 0;

        freq = C4;
        ticks = TICKS_PER_SECOND/freq/SQUARE_SAMPLES;
		amplitude_scale_x = 100; // zapnutie hlasitosti a zahratie dekodovaneho tonu z klavesnice		
		delay_ms(150);
		amplitude_scale_x = 0;

        delay_ms(150);

        freq = A3;
        ticks = TICKS_PER_SECOND/freq/SQUARE_SAMPLES;
		amplitude_scale_x = 100; // zapnutie hlasitosti a zahratie dekodovaneho tonu z klavesnice		
		delay_ms(150);
		amplitude_scale_x = 0;

        freq = C4;
        ticks = TICKS_PER_SECOND/freq/SQUARE_SAMPLES;
		amplitude_scale_x = 100; // zapnutie hlasitosti a zahratie dekodovaneho tonu z klavesnice		
		delay_ms(150);
		amplitude_scale_x = 0;

        freq = D4;
        ticks = TICKS_PER_SECOND/freq/SQUARE_SAMPLES;
		amplitude_scale_x = 100; // zapnutie hlasitosti a zahratie dekodovaneho tonu z klavesnice		
		delay_ms(150);
		amplitude_scale_x = 0;
    }

    void part5_demo() {
        delay_ms(300);
 
        freq = G4;
        ticks = TICKS_PER_SECOND/freq/SQUARE_SAMPLES;
		amplitude_scale_x = 100; // zapnutie hlasitosti a zahratie dekodovaneho tonu z klavesnice		
		delay_ms(150);
		amplitude_scale_x = 0;

        freq = FS4;
        ticks = TICKS_PER_SECOND/freq/SQUARE_SAMPLES;
		amplitude_scale_x = 100; // zapnutie hlasitosti a zahratie dekodovaneho tonu z klavesnice		
		delay_ms(150);
		amplitude_scale_x = 0;

        freq = F4;
        ticks = TICKS_PER_SECOND/freq/SQUARE_SAMPLES;
		amplitude_scale_x = 100; // zapnutie hlasitosti a zahratie dekodovaneho tonu z klavesnice		
		delay_ms(150);
		amplitude_scale_x = 0;

        freq = EB4;
        ticks = TICKS_PER_SECOND/freq/SQUARE_SAMPLES;
		amplitude_scale_x = 100; // zapnutie hlasitosti a zahratie dekodovaneho tonu z klavesnice		
		delay_ms(150);
		amplitude_scale_x = 0;

        delay_ms(150);

        freq = E4;
        ticks = TICKS_PER_SECOND/freq/SQUARE_SAMPLES;
		amplitude_scale_x = 100; // zapnutie hlasitosti a zahratie dekodovaneho tonu z klavesnice		
		delay_ms(150);
		amplitude_scale_x = 0;

        delay_ms(150);

        freq = C5;
        ticks = TICKS_PER_SECOND/freq/SQUARE_SAMPLES;
		amplitude_scale_x = 100; // zapnutie hlasitosti a zahratie dekodovaneho tonu z klavesnice		
		delay_ms(150);
		amplitude_scale_x = 0;

        delay_ms(150);

        freq = C5;
        ticks = TICKS_PER_SECOND/freq/SQUARE_SAMPLES;
		amplitude_scale_x = 100; // zapnutie hlasitosti a zahratie dekodovaneho tonu z klavesnice		
		delay_ms(150);
		amplitude_scale_x = 0;

        freq = C5;
        ticks = TICKS_PER_SECOND/freq/SQUARE_SAMPLES;
		amplitude_scale_x = 100; // zapnutie hlasitosti a zahratie dekodovaneho tonu z klavesnice		
		delay_ms(150);
		amplitude_scale_x = 0;

        delay_ms(450);
    }

    void part6_demo() {
        delay_ms(300);
        
        freq = EB4;
        ticks = TICKS_PER_SECOND/freq/SQUARE_SAMPLES;
		amplitude_scale_x = 100; // zapnutie hlasitosti a zahratie dekodovaneho tonu z klavesnice		
		delay_ms(150);
		amplitude_scale_x = 0;

        delay_ms(300);

        freq = D4;
        ticks = TICKS_PER_SECOND/freq/SQUARE_SAMPLES;
		amplitude_scale_x = 100; // zapnutie hlasitosti a zahratie dekodovaneho tonu z klavesnice		
		delay_ms(150);
		amplitude_scale_x = 0;

        delay_ms(300);

        freq = C4;
        ticks = TICKS_PER_SECOND/freq/SQUARE_SAMPLES;
		amplitude_scale_x = 100; // zapnutie hlasitosti a zahratie dekodovaneho tonu z klavesnice		
		delay_ms(150);
		amplitude_scale_x = 0;

        delay_ms(1050);
    }


    void part7_demo() {
        freq = C4;
        ticks = TICKS_PER_SECOND/freq/SQUARE_SAMPLES;
		amplitude_scale_x = 100; // zapnutie hlasitosti a zahratie dekodovaneho tonu z klavesnice		
		delay_ms(150);
		amplitude_scale_x = 0;

        freq = C4;
        ticks = TICKS_PER_SECOND/freq/SQUARE_SAMPLES;
		amplitude_scale_x = 100; // zapnutie hlasitosti a zahratie dekodovaneho tonu z klavesnice		
		delay_ms(150);
		amplitude_scale_x = 0;

        delay_ms(150);

        freq = C4;
        ticks = TICKS_PER_SECOND/freq/SQUARE_SAMPLES;
		amplitude_scale_x = 100; // zapnutie hlasitosti a zahratie dekodovaneho tonu z klavesnice		
		delay_ms(150);
		amplitude_scale_x = 0;

        delay_ms(150);

        freq = C4;
        ticks = TICKS_PER_SECOND/freq/SQUARE_SAMPLES;
		amplitude_scale_x = 100; // zapnutie hlasitosti a zahratie dekodovaneho tonu z klavesnice		
		delay_ms(150);
		amplitude_scale_x = 0;

        freq = D4;
        ticks = TICKS_PER_SECOND/freq/SQUARE_SAMPLES;
		amplitude_scale_x = 100; // zapnutie hlasitosti a zahratie dekodovaneho tonu z klavesnice		
		delay_ms(150);
		amplitude_scale_x = 0;

        delay_ms(150);

        freq = E4;
        ticks = TICKS_PER_SECOND/freq/SQUARE_SAMPLES;
		amplitude_scale_x = 100; // zapnutie hlasitosti a zahratie dekodovaneho tonu z klavesnice		
		delay_ms(150);
		amplitude_scale_x = 0;

        freq = C4;
        ticks = TICKS_PER_SECOND/freq/SQUARE_SAMPLES;
		amplitude_scale_x = 100; // zapnutie hlasitosti a zahratie dekodovaneho tonu z klavesnice		
		delay_ms(150);
		amplitude_scale_x = 0;

        delay_ms(150);

        freq = A3;
        ticks = TICKS_PER_SECOND/freq/SQUARE_SAMPLES;
		amplitude_scale_x = 100; // zapnutie hlasitosti a zahratie dekodovaneho tonu z klavesnice		
		delay_ms(150);
		amplitude_scale_x = 0;

        freq = G3;
        ticks = TICKS_PER_SECOND/freq/SQUARE_SAMPLES;
		amplitude_scale_x = 100; // zapnutie hlasitosti a zahratie dekodovaneho tonu z klavesnice		
		delay_ms(150);
		amplitude_scale_x = 0;

        delay_ms(450);
    }

    void part8_demo(){
        freq = C4;
        ticks = TICKS_PER_SECOND/freq/SQUARE_SAMPLES;
		amplitude_scale_x = 100; // zapnutie hlasitosti a zahratie dekodovaneho tonu z klavesnice		
		delay_ms(150);
		amplitude_scale_x = 0;

        freq = C4;
        ticks = TICKS_PER_SECOND/freq/SQUARE_SAMPLES;
		amplitude_scale_x = 100; // zapnutie hlasitosti a zahratie dekodovaneho tonu z klavesnice		
		delay_ms(150);
		amplitude_scale_x = 0;

        delay_ms(150);

        freq = C4;
        ticks = TICKS_PER_SECOND/freq/SQUARE_SAMPLES;
		amplitude_scale_x = 100; // zapnutie hlasitosti a zahratie dekodovaneho tonu z klavesnice		
		delay_ms(150);
		amplitude_scale_x = 0;

        delay_ms(150);

        freq = C4;
        ticks = TICKS_PER_SECOND/freq/SQUARE_SAMPLES;
		amplitude_scale_x = 100; // zapnutie hlasitosti a zahratie dekodovaneho tonu z klavesnice		
		delay_ms(150);
		amplitude_scale_x = 0;

        freq = D4;
        ticks = TICKS_PER_SECOND/freq/SQUARE_SAMPLES;
		amplitude_scale_x = 100; // zapnutie hlasitosti a zahratie dekodovaneho tonu z klavesnice		
		delay_ms(150);
		amplitude_scale_x = 0;

        freq = E4;
        ticks = TICKS_PER_SECOND/freq/SQUARE_SAMPLES;
		amplitude_scale_x = 100; // zapnutie hlasitosti a zahratie dekodovaneho tonu z klavesnice		
		delay_ms(150);
		amplitude_scale_x = 0;

        delay_ms(1050);
    }
    
    void play_demo() {
        part1_demo();
        part2_demo();
        part3_demo();
        part4_demo();
        part5_demo();
        part6_demo();
        part7_demo();
        part8_demo();
    }

interrupt (TIMERA0_VECTOR) Timer_A (void)
{ 	
		// ABY TO HRALO MUSI TO "KMITAT", takze striedam urovne 255 0
		if(sample_iterator == 2) {sample_iterator = 0;} 
		
		/*
	        * reagujem na prerusenie z klavesnice, aby som dosiahol ziadaneho efektu hudonneho nastroja
		* takze spracovavane data prevodnikom musia
		*/
		DAC12_0DAT = (arr_square[sample_iterator]*amplitude_scale_x)/100; // nahratie dalsieho vzorku pre prevod
		CCR0 += ticks; // pocet tikov po ktorych pride k dalsiemu preruseniu a nasledne prevodu 
		sample_iterator++;
	
}

void print_user_help(void)
{         
	term_send_str_crlf("Ovladanie hudnobneho simulatoru");
	term_send_str_crlf("Ovladanie klavesnice");
	
    // jednociarkova oktava
	term_send_str_crlf(">-klavesa '1' zahra ton C4(c')");
	term_send_str_crlf(">-klavesa '2' zahra ton D4(d')");
	term_send_str_crlf(">-klavesa '3' zahra ton E4(e')");
	term_send_str_crlf(">-klavesa 'A' zahra ton F4(f')");
	term_send_str_crlf(">-klavesa '4' zahra ton G4(g')");
	term_send_str_crlf(">-klavesa '5' zahra ton A4(a')");
	term_send_str_crlf(">-klavesa '6' zahra ton B4(h')");
	
    // dvojciarkova oktava
	term_send_str_crlf(">-klavesa '7' zahra ton C5(c'')");
	term_send_str_crlf(">-klavesa '8' zahra ton D5(d'')");
	term_send_str_crlf(">-klavesa '9' zahra ton E5(e'')");
	term_send_str_crlf(">-klavesa 'C' zahra ton F5(f'')");
	term_send_str_crlf(">-klavesa '*' zahra ton G5(g'')");
	term_send_str_crlf(">-klavesa '0' zahra ton A5(a'')");
	term_send_str_crlf(">-klavesa '6' zahra ton B4(h'')");
	 
	// song
	term_send_str_crlf(">-klavesa 'D' a prehra demo skladbu");


	term_send_str_crlf("Ovladanie terminalom");
	// jednociarkova oktava
	term_send_str_crlf(">-zadaj prikaz 'C4' a zahra sa ton C4(c')");
	term_send_str_crlf(">-zadaj prikaz 'D4' a zahra sa ton D4(d')");
	term_send_str_crlf(">-zadaj prikaz 'E4' a zahra sa ton E4(e')");
	term_send_str_crlf(">-zadaj prikaz 'F4' a zahra sa ton F4(f')");
	term_send_str_crlf(">-zadaj prikaz 'G4' a zahra sa ton G4(g')");
	term_send_str_crlf(">-zadaj prikaz 'A4' a zahra sa ton A4(a')");
	term_send_str_crlf(">-zadaj prikaz 'B4' a zahra sa ton B4(h')");
	
    // dvojciarkova oktava
	term_send_str_crlf(">-zadaj prikaz 'C5' a zahra sa ton C5(c'')");
	term_send_str_crlf(">-zadaj prikaz 'D5' a zahra sa ton D5(d'')");
	term_send_str_crlf(">-zadaj prikaz 'E5' a zahra sa ton E5(e'')");
	term_send_str_crlf(">-zadaj prikaz 'F5' a zahra sa ton F5(f'')");
	term_send_str_crlf(">-zadaj prikaz 'G5' a zahra sa ton G5(g'')");
	term_send_str_crlf(">-zadaj prikaz 'A5' a zahra sa ton A5(a'')");
	term_send_str_crlf(">-zadaj prikaz 'B4' a zahra sa ton B4(h'')");
	 
	// song
	term_send_str_crlf(">-zadaj prikaz 'DEMO' a prehra demo skladbu");
}

// Incializacia periferii
void fpga_initialized()
{ 
    LCD_init();
    LCD_write_string("Simulator hudby");
	
    // poslanie infa do terminalu
    term_send_str_crlf(" ");   
    term_send_str_crlf("SIMULATOR HUDBY POMOCOU DA PREVODNIKA");
    term_send_str_crlf("Napetie je generovane na 0. kanálu DA prevodniku mikrokontrolera.");
    term_send_str_crlf("Prosim pripojte svoj reproduktor na pin c. 31 JP9 pinoveho pola a GND pin");
    term_send_str_crlf("Ovladanie je mozne pomocou klavesnice aj terminalu");
    term_send_str_crlf("Napiste 'help' pre popis ovladania");

}

// Dekodovanie prikazov uzivatela  v terminale
unsigned char decode_user_cmd(char *UserCommand, char *ComparedCommand) 
{   
    	if (strcmp2(UserCommand, "C4"))
        {
         	LCD_write_string("Ton: C4 (c')");// vycisti obrazovku a zapis retazec na displej fitkitu
            freq = C4;
			ticks = TICKS_PER_SECOND/freq/SQUARE_SAMPLES;
			amplitude_scale_x = 100; // zapnutie hlasitosti a zahratie dekodovaneho tonu z klavesnice
			delay_ms(300);
			amplitude_scale_x = 0;
		
		}
		else if (strcmp2(UserCommand, "D4"))
        {
        	LCD_write_string("Ton: D4 (d')");// vycisti obrazovku a zapis retazec na displej fitkitu
			freq = D4;
            ticks = TICKS_PER_SECOND/freq/SQUARE_SAMPLES;
			amplitude_scale_x = 100; // zapnutie hlasitosti a zahratie dekodovaneho tonu z klavesnice
			delay_ms(300);
			amplitude_scale_x = 0;		
		}
		else if (strcmp2(UserCommand, "E4"))
        {
         	LCD_write_string("Ton: E4 (e')");// vycisti obrazovku a zapis retazec na displej fitkitu
			freq = E4;
            ticks = TICKS_PER_SECOND/freq/SQUARE_SAMPLES;
			amplitude_scale_x = 100; // zapnutie hlasitosti a zahratie dekodovaneho tonu z klavesnice		
			delay_ms(300);
			amplitude_scale_x = 0;
		}
		
		else if (strcmp2(UserCommand, "F4"))
        {
         	LCD_write_string("Ton: F4 (f')");// vycisti obrazovku a zapis retazec na displej fitkitu
			freq = F4;
            ticks = TICKS_PER_SECOND/freq/SQUARE_SAMPLES;
			amplitude_scale_x = 100; // zapnutie hlasitosti a zahratie dekodovaneho tonu z klavesnice		
			delay_ms(300);
			amplitude_scale_x = 0;
		}		
		
		else if (strcmp2(UserCommand, "G4"))
        { 
         	LCD_write_string("Ton: G4 (g')");// vycisti obrazovku a zapis retazec na displej fitkitu
			freq = G4;
            ticks = TICKS_PER_SECOND/freq/SQUARE_SAMPLES;
			amplitude_scale_x = 100; // zapnutie hlasitosti a zahratie dekodovaneho tonu z klavesnice		
			delay_ms(300);
			amplitude_scale_x = 0;
		}
		
		else if (strcmp2(UserCommand, "A4"))
        { 
         	LCD_write_string("Ton: A4 (a')");// vycisti obrazovku a zapis retazec na displej fitkitu
			freq = A4;
            ticks = TICKS_PER_SECOND/freq/SQUARE_SAMPLES;
			amplitude_scale_x = 100; // zapnutie hlasitosti a zahratie dekodovaneho tonu z klavesnice		
			delay_ms(300);
			amplitude_scale_x = 0;
		}		

		else if (strcmp2(UserCommand, "B4"))
        { 
         	LCD_write_string("Ton: B4 (h')");// vycisti obrazovku a zapis retazec na displej fitkitu
			freq = B4;
            ticks = TICKS_PER_SECOND/freq/SQUARE_SAMPLES;
			amplitude_scale_x = 100; // zapnutie hlasitosti a zahratie dekodovaneho tonu z klavesnice		
			delay_ms(300);
			amplitude_scale_x = 0;		
		
		}
    
		else if (strcmp2(UserCommand, "C5"))
        {
         	LCD_write_string("Ton: C5 (c'')");// vycisti obrazovku a zapis retazec na displej fitkitu
			freq = C5;
            ticks = TICKS_PER_SECOND/freq/SQUARE_SAMPLES;
			amplitude_scale_x = 100; // zapnutie hlasitosti a zahratie dekodovaneho tonu z klavesnice		
			delay_ms(300);
			amplitude_scale_x = 0;
		}

		else if (strcmp2(UserCommand, "D5"))
        {
        	LCD_write_string("Ton: D5 (d'')");// vycisti obrazovku a zapis retazec na displej fitkitu
			freq = D5;
            ticks = TICKS_PER_SECOND/freq/SQUARE_SAMPLES;
			amplitude_scale_x = 100; // zapnutie hlasitosti a zahratie dekodovaneho tonu z klavesnice		
			delay_ms(300);
			amplitude_scale_x = 0;
		}

		else if (strcmp2(UserCommand, "E5"))
        { 
         	LCD_write_string("Ton: E5 (e'')");// vycisti obrazovku a zapis retazec na displej fitkitu
			freq = E5;
            ticks = TICKS_PER_SECOND/freq/SQUARE_SAMPLES;
			amplitude_scale_x = 100; // zapnutie hlasitosti a zahratie dekodovaneho tonu z klavesnice		
			delay_ms(300);
			amplitude_scale_x = 0;
		}
		
		else if (strcmp2(UserCommand, "F5"))
        { 
         	LCD_write_string("Ton: F5 (f'')");// vycisti obrazovku a zapis retazec na displej fitkitu
			freq = F5;
            ticks = TICKS_PER_SECOND/freq/SQUARE_SAMPLES;
			amplitude_scale_x = 100; // zapnutie hlasitosti a zahratie dekodovaneho tonu z klavesnice		
			delay_ms(300);
			amplitude_scale_x = 0;
		}		
		
		else if (strcmp2(UserCommand, "G5"))
        { 
         	LCD_write_string("Ton: G5 (g'')");// vycisti obrazovku a zapis retazec na displej fitkitu
			freq = G5;
            ticks = TICKS_PER_SECOND/freq/SQUARE_SAMPLES;
			amplitude_scale_x = 100; // zapnutie hlasitosti a zahratie dekodovaneho tonu z klavesnice		
			delay_ms(300);
			amplitude_scale_x = 0;
		}
		
		else if (strcmp2(UserCommand, "A5")) 
        {     
    	    LCD_write_string("Ton: A5 (a'')");// vycisti obrazovku a zapis retazec na displej fitkitu
			freq = A5;
            ticks = TICKS_PER_SECOND/freq/SQUARE_SAMPLES;
			amplitude_scale_x = 100; // zapnutie hlasitosti a zahratie dekodovaneho tonu z klavesnice		
			delay_ms(300);
			amplitude_scale_x = 0;
		}		

		else if (strcmp2(UserCommand, "B5")) 
        { 
        	LCD_write_string("Ton: B5 (h'')");// vycisti obrazovku a zapis retazec na displej fitkitu
			freq = B5;
            ticks = TICKS_PER_SECOND/freq/SQUARE_SAMPLES;
			amplitude_scale_x = 100; // zapnutie hlasitosti a zahratie dekodovaneho tonu z klavesnice		
			delay_ms(300);
			amplitude_scale_x = 0;
		}
        else if (strcmp4(UserCommand, "DEMO")) 
        { 
        	LCD_write_string("Hra DEMO skladba");// vycisti obrazovku a zapis retazec na displej fitkitu
            play_demo();
		}
        else 
        {
            return (CMD_UNKNOWN);
        }
        return USER_COMMAND; 
    
}


char tone_decoder(unsigned int keyboard_input)
{	// jednociarkova oktava

	// C4
	if (keyboard_input & KEY_1)
       	{
		freq = C4;
		return 'C';
	}

	// D4
	if (keyboard_input & KEY_2)
       	{
		freq = D4;
		return 'D';
	}

	// E4
	if (keyboard_input & KEY_3)
       	{
		freq = E4;
		return 'E';
	}

	// F4
	if (keyboard_input & KEY_A)
       	{
		freq = F4;
		return 'F';
	}

	// G4
	if (keyboard_input & KEY_4)
       	{
		freq = G4;
		return 'G';
	}

	// A4
	if (keyboard_input & KEY_5)
       	{
		freq = A4;
		return 'A';
	}

	//  B4
	if (keyboard_input & KEY_6)
       	{
		freq = B4;
		return 'B';
	}

	// C5
	if (keyboard_input & KEY_7)
       	{
		freq = C5;
		return '1';
	}

	// D5
	if (keyboard_input & KEY_8)
       	{
		freq = D5;
		return '2';
	}

	// E5
	if (keyboard_input & KEY_9)
       	{
		freq = E5;
		return '3';
	}

	// F5
	if (keyboard_input & KEY_C)
       	{
		freq = F5;
		return '4';
	}

	// G5
	if (keyboard_input & KEY_h)
       	{
		freq = G5;
		return '5';
	}

	// A5
	if (keyboard_input & KEY_0)
       	{
		freq = A5;
		return '6';
	}

	//  B5
	if (keyboard_input & KEY_m)
       	{
		freq = B5;
		return '7';
	}

	//  DEMO skladba 
	if (keyboard_input & KEY_D)
    {
		return '8';
	}
	return PROCESS_OK;
}



int keyboard_idle()
{
    char ch;

    ch = tone_decoder(read_word_keyboard_4x4());

        if (ch != 0)
        {
		
		if (ch == 'C') {
         		LCD_write_string("Ton: C4 (c')");// vycisti obrazovku a zapis retazec na displej fitkitu
			ticks = TICKS_PER_SECOND/freq/SQUARE_SAMPLES;
			amplitude_scale_x = 100; // zapnutie hlasitosti a zahratie dekodovaneho tonu z klavesnice
			delay_ms(300);
			amplitude_scale_x = 0;
		
		}
		if (ch == 'D') {
        		LCD_write_string("Ton: D4 (d')");// vycisti obrazovku a zapis retazec na displej fitkitu
			ticks = TICKS_PER_SECOND/freq/SQUARE_SAMPLES;
			amplitude_scale_x = 100; // zapnutie hlasitosti a zahratie dekodovaneho tonu z klavesnice
			delay_ms(300);
			amplitude_scale_x = 0;		
		}
		if (ch == 'E') {
         	LCD_write_string("Ton: E4 (e')");// vycisti obrazovku a zapis retazec na displej fitkitu
			ticks = TICKS_PER_SECOND/freq/SQUARE_SAMPLES;
			amplitude_scale_x = 100; // zapnutie hlasitosti a zahratie dekodovaneho tonu z klavesnice		
			delay_ms(300);
			amplitude_scale_x = 0;
		}
		
		if (ch == 'F') { 
         	LCD_write_string("Ton: F4 (f')");// vycisti obrazovku a zapis retazec na displej fitkitu
			ticks = TICKS_PER_SECOND/freq/SQUARE_SAMPLES;
			amplitude_scale_x = 100; // zapnutie hlasitosti a zahratie dekodovaneho tonu z klavesnice		
			delay_ms(300);
			amplitude_scale_x = 0;
		}		
		
		if (ch == 'G') { 
         	LCD_write_string("Ton: G4 (g')");// vycisti obrazovku a zapis retazec na displej fitkitu
			ticks = TICKS_PER_SECOND/freq/SQUARE_SAMPLES;
			amplitude_scale_x = 100; // zapnutie hlasitosti a zahratie dekodovaneho tonu z klavesnice		
			delay_ms(300);
			amplitude_scale_x = 0;
		}
		
		if (ch == 'A') {
         	LCD_write_string("Ton: A4 (a')");// vycisti obrazovku a zapis retazec na displej fitkitu
			ticks = TICKS_PER_SECOND/freq/SQUARE_SAMPLES;
			amplitude_scale_x = 100; // zapnutie hlasitosti a zahratie dekodovaneho tonu z klavesnice		
			delay_ms(300);
			amplitude_scale_x = 0;
		}		

		if (ch == 'B') { 
         	LCD_write_string("Ton: B4 (h')");// vycisti obrazovku a zapis retazec na displej fitkitu
			ticks = TICKS_PER_SECOND/freq/SQUARE_SAMPLES;
			amplitude_scale_x = 100; // zapnutie hlasitosti a zahratie dekodovaneho tonu z klavesnice		
			delay_ms(300);
			amplitude_scale_x = 0;		
		
		}
    
		if (ch == '1') {
         	LCD_write_string("Ton: C5 (c'')");// vycisti obrazovku a zapis retazec na displej fitkitu
			ticks = TICKS_PER_SECOND/freq/SQUARE_SAMPLES;
			amplitude_scale_x = 100; // zapnutie hlasitosti a zahratie dekodovaneho tonu z klavesnice		
			delay_ms(300);
			amplitude_scale_x = 0;
		}

		if (ch == '2') {
        	LCD_write_string("Ton: D5 (d'')");// vycisti obrazovku a zapis retazec na displej fitkitu
			ticks = TICKS_PER_SECOND/freq/SQUARE_SAMPLES;
			amplitude_scale_x = 100; // zapnutie hlasitosti a zahratie dekodovaneho tonu z klavesnice		
			delay_ms(300);
			amplitude_scale_x = 0;
		}

		if (ch == '3') { 
         	LCD_write_string("Ton: E5 (e'')");// vycisti obrazovku a zapis retazec na displej fitkitu
			ticks = TICKS_PER_SECOND/freq/SQUARE_SAMPLES;
			amplitude_scale_x = 100; // zapnutie hlasitosti a zahratie dekodovaneho tonu z klavesnice		
			delay_ms(300);
			amplitude_scale_x = 0;
		}
		
		if (ch == '4') { 
         	LCD_write_string("Ton: F5 (f'')");// vycisti obrazovku a zapis retazec na displej fitkitu
			ticks = TICKS_PER_SECOND/freq/SQUARE_SAMPLES;
			amplitude_scale_x = 100; // zapnutie hlasitosti a zahratie dekodovaneho tonu z klavesnice		
			delay_ms(300);
			amplitude_scale_x = 0;
		}		
		
		if (ch == '5') {
         	LCD_write_string("Ton: G5 (g'')");// vycisti obrazovku a zapis retazec na displej fitkitu
			ticks = TICKS_PER_SECOND/freq/SQUARE_SAMPLES;
			amplitude_scale_x = 100; // zapnutie hlasitosti a zahratie dekodovaneho tonu z klavesnice		
			delay_ms(300);
			amplitude_scale_x = 0;
		}
		
		if (ch == '6') { 
         	LCD_write_string("Ton: A5 (a'')");// vycisti obrazovku a zapis retazec na displej fitkitu
			ticks = TICKS_PER_SECOND/freq/SQUARE_SAMPLES;
			amplitude_scale_x = 100; // zapnutie hlasitosti a zahratie dekodovaneho tonu z klavesnice		
			delay_ms(300);
			amplitude_scale_x = 0;
		}		

		if (ch == '7') { 
         	LCD_write_string("Ton: B5 (h'')");// vycisti obrazovku a zapis retazec na displej fitkitu
			ticks = TICKS_PER_SECOND/freq/SQUARE_SAMPLES;
			amplitude_scale_x = 100; // zapnutie hlasitosti a zahratie dekodovaneho tonu z klavesnice		
			delay_ms(300);
			amplitude_scale_x = 0;
		}
        if (ch == '8') { 
         	LCD_write_string("Hra DEMO skladba");// vycisti obrazovku a zapis retazec na displej fitkitu
            play_demo();
		}
    		
	}

    
    

return PROCESS_OK;
    
}

