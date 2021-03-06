
/* 
 * Argyll Color Correction System
 *
 * HCFR Association HCFR sensor related functions
 *
 * Author: Graeme W. Gill
 * Date:   20/1/2007
 *
 * Copyright 2007, Graeme W. Gill
 * All rights reserved.
 *
 * This material is licenced under the GNU AFFERO GENERAL PUBLIC LICENSE Version 3 :-
 * see the License.txt file for licencing details.
 */

/* 
   If you make use of the instrument driver code here, please note
   that it is the author(s) of the code who take responsibility
   for its operation. Any problems or queries regarding driving
   instruments with the Argyll drivers, should be directed to
   the Argyll's author(s), and not to any other party.

   If there is some instrument feature or function that you
   would like supported here, it is recommended that you
   contact Argyll's author(s) first, rather than attempt to
   modify the software yourself, if you don't have firm knowledge
   of the instrument communicate protocols. There is a chance
   that an instrument could be damaged by an incautious command
   sequence, and the instrument companies generally cannot and
   will not support developers that they have not qualified
   and agreed to support.
 */

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <time.h>
#include <stdarg.h>
#include "copyright.h"
#include "config.h"
#include "numlib.h"
#include "xspect.h"
#include "insttypes.h"
#include "icoms.h"
#include "conv.h"
#include "hcfr.h"

#undef MEASURE_RAW		/* To facilitie calibration - return raw RGB values */
#undef DEBUG

static inst_code hcfr_interp_code(inst *pp, int ec);

#define MAX_MES_SIZE 500		/* Maximum normal message reply size */
#define MAX_RD_SIZE 5000		/* Maximum reading messagle reply size */

/* ==================================================================== */

/* Interpret an icoms error into a HCFR error */
static int icoms2hcfr_err(int se) {
	if (se & ICOM_USERM) {
		se &= ICOM_USERM;
		if (se == ICOM_USER)
			return HCFR_USER_ABORT;
		if (se == ICOM_TERM)
			return HCFR_USER_TERM;
		if (se == ICOM_TRIG)
			return HCFR_USER_TRIG;
		if (se == ICOM_CMND)
			return HCFR_USER_CMND;
	}
	if (se != ICOM_OK)
		return HCFR_COMS_FAIL;
	return HCFR_OK;
}

/* Do a standard command/response echange with the hcfr */
/* Return the dtp error code */
static inst_code
hcfr_command(
	hcfr *p,
	char *in,		/* In string */
	char *out,		/* Out string */
	int bsize,		/* Out buffer size */
	double to		/* Timeout in seconds */
) {
	int rv, se;

	if ((se = p->icom->write_read(p->icom, in, out, bsize, '\n', 1, to)) != 0) {
		int ec;
#ifdef DEBUG
		printf("hcfr fcommand: serial i/o failure on write_read '%s'\n",icoms_fix(in));
#endif
		return hcfr_interp_code((inst *)p, icoms2hcfr_err(se));
	}
#ifdef DEBUG
	printf("command '%s'",icoms_fix(in));
	printf(" returned '%s', value 0x%x\n",icoms_fix(out),HCFR_OK);
#endif
	return hcfr_interp_code((inst *)p, HCFR_OK);
}

/* Do a break to check coms is working */
inst_code
hcfr_break(
	hcfr *p
) {
	int rwbytes;			/* Data bytes read or written */
	int se, rv = inst_ok;
	int isdeb = 0;

	/* Turn off low level debug messages, and sumarise them here */
	isdeb = p->icom->debug;
	p->icom->debug = 0;

	if (isdeb) printf("\nhcfr: Doing break\n");

	se = p->icom->usb_control(p->icom,
		               USB_ENDPOINT_OUT | USB_TYPE_CLASS | USB_RECIP_INTERFACE,
	                   0x22, 0, 0, NULL, 0, 1.0);

	rv = hcfr_interp_code((inst *)p, icoms2hcfr_err(se));

	if (isdeb) printf("Break done, ICOM err 0x%x\n",se);
	p->icom->debug = isdeb;

	return rv;
}

/* Flush an pending messages from the device */
inst_code
hcfr_flush(
	hcfr *p
) {
	icoms *c = p->icom;
	char buf[MAX_MES_SIZE];
	inst_code ev = inst_ok;

	for (c->lerr = 0;;) {
		int debug = c->debug; c->debug = 0;
		c->read(c, buf, MAX_MES_SIZE, '\000', 100000, 0.01);
		c->debug = debug;
		if (c->lerr != 0)
			break;				/* Expect timeout with nothing to read */
	}
	c->lerr = 0;
		
	return inst_ok;
}

/* Get and check the firmware version */
inst_code
hcfr_get_check_version(
	hcfr *p,
	int *pmaj,
	int *pmin
) {
	char ibuf[2];
	char buf[MAX_MES_SIZE];
	inst_code ev = inst_ok;
	int maj, min;

	if (p->debug) fprintf(stderr,"hcfr: About to read firmware version\n");

	if (p->gotcoms == 0)
		return inst_internal_error;

	ibuf[0] = HCFR_GET_VERS;
	ibuf[1] = 0x00;

	if ((ev = hcfr_command(p, ibuf, buf, MAX_MES_SIZE, 1.0)) != inst_ok) {
		if (p->debug) fprintf(stderr,"hcfr_command failed\n");
		return ev;
	}

	if (strlen(buf) < 6) {
		if (p->debug) fprintf(stderr,"version string too short\n");
		return hcfr_interp_code((inst *)p, HCFR_BAD_FIRMWARE);
	}

	if (sscanf(buf, "v%d.%d", &maj,&min) != 2) {
		if (p->debug) fprintf(stderr,"version string doesn't match format\n");
		return hcfr_interp_code((inst *)p, HCFR_BAD_FIRMWARE);
	}

	if (maj != HCFR_FIRMWARE_MAJOR_VERSION || min < HCFR_FIRMWARE_MINOR_VERSION) {
		if (p->debug) fprintf(stderr,"version string out of range\n");
		return hcfr_interp_code((inst *)p, HCFR_BAD_FIRMWARE);
	}

	if (p->debug) fprintf(stderr,"hcfr: Got firmare version %d.%d\n",maj,min);
	if (pmaj != NULL)
		*pmaj = maj;
	if (pmin != NULL)
		*pmin = min;
		
	return inst_ok;
}

/* Get a raw measurement value */
inst_code
hcfr_get_rgb(
	hcfr *p,
	double rgb[3]	/* return value */
) {
	char ibuf[2];
	char buf[MAX_MES_SIZE], *bp;
	char vbuf[4];
	inst_code ev = inst_ok;
	double mul, div;
	double vals[8];
	int onesens = 0;
	int i;

	if (p->debug) fprintf(stderr,"hcfr: About to read RGB value\n");

	if (p->gotcoms == 0)
		return inst_internal_error;

	ibuf[0] = HCFR_MEAS_RGB			/* Read RGB */
	        | HCFR_MEAS_SENS0		/* Use one sensors because it's faster */
//	        | HCFR_MEAS_SENS1
	        | HCFR_INTERLACE_0
//	        | HCFR_FAST_MEASURE
	        ;

	ibuf[1] = 0x00;

	if ((ev = hcfr_command(p, ibuf, buf, MAX_MES_SIZE, 60.0)) != inst_ok) {
		if (p->debug) fprintf(stderr,"hcfr_command failed\n");
		return ev;
	}

	if (strlen(buf) < 156)
		return hcfr_interp_code((inst *)p, HCFR_BAD_READING);
		
	if (strncmp(buf, "RGB_1:", 6) == 0)
		onesens = 1;
	else 
	if (strncmp(buf, "RGB_2:", 6) != 0)
		return hcfr_interp_code((inst *)p, HCFR_BAD_READING);

	vbuf[3] = 0x00;
	bp = buf + 6;
	
	strncpy(vbuf, bp, 3); div = (double)atoi(vbuf); bp += 3;

	strncpy(vbuf, bp, 3); mul = (double)atoi(vbuf); bp += 3;
	
	/* Compute all the possible values for 4 colors and 2 sensors */
	for (i = 0; i < 8; i++) {
		unsigned int num, den;

		strncpy(vbuf, bp, 3); den =              atoi(vbuf); bp += 3;
		strncpy(vbuf, bp, 3); den = (den << 8) + atoi(vbuf); bp += 3;
		strncpy(vbuf, bp, 3); den = (den << 8) + atoi(vbuf); bp += 3;
		strncpy(vbuf, bp, 3); den = (den << 8) + atoi(vbuf); bp += 3;

		strncpy(vbuf, bp, 3); num =              atoi(vbuf); bp += 3;
		strncpy(vbuf, bp, 3); num = (num << 8) + atoi(vbuf); bp += 3;

		if (den == 0)		/* Hmm. */
			vals[i] = -1.0;
		else
			vals[i] = 1e6 * (double)num * mul * div / (double)den;
//printf("~1 vals[%d] = %f = num %d * mul %f * div %f / den %d\n", i, vals[i], num,mul,div,den);
	}
	if (onesens) {
		rgb[0] = vals[0];
		rgb[1] = vals[1];
		rgb[2] = vals[2];
	} else {
//printf("~1 diff = %f%%\n",100.0 * vals[0]/vals[4]);
//printf("~1 diff = %f%%\n",100.0 * vals[1]/vals[5]);
//printf("~1 diff = %f%%\n",100.0 * vals[2]/vals[6]);
		rgb[0] = 0.5 * (vals[0] + vals[4]);
		rgb[1] = 0.5 * (vals[1] + vals[5]);
		rgb[2] = 0.5 * (vals[2] + vals[6]);
	}
//printf("~1 done hcfr_get_rgb, about to return values %f %f %f\n",rgb[0],rgb[1],rgb[2]);

	return inst_ok;
}

/* Compute the calibration matricies. */
/* The basic calibration data is from my particular HCFR, measured */
/* against one of my CRT and LCD displays, with the reference XYZ */
/* derived from my i1pro. */
inst_code
hcfr_comp_matrix(
	hcfr *p
) {
	double tmat[3][3];
	double xmat[3][3];
	double itmat[3][3];

	/* CRT */

	/* Red test patch, sensor then reference */
	tmat[0][0] = 7171.880890;
	tmat[1][0] = 853.740337;
	tmat[2][0] = 308.216218;

	xmat[0][0] = 21.988601;
	xmat[1][0] = 12.131219;
	xmat[2][0] = 1.312786;

	/* Green test patch, sensor then reference */
	tmat[0][1] = 626.299108; 
	tmat[1][1] = 3749.843127;
	tmat[2][1] = 1591.104086;

	xmat[0][1] = 13.677691;
	xmat[1][1] = 28.870823;
	xmat[2][1] = 5.636190;

	/* Blue test patch, sensor then reference */
	tmat[0][2] = 130.620298; 
	tmat[1][2] = 462.894673;
	tmat[2][2] = 2757.654019;

	xmat[0][2] = 6.387302;
	xmat[1][2] = 2.755360;
	xmat[2][2] = 33.588242;

	/* Compute the inverse */
	if (icmInverse3x3(itmat, tmat))
		return hcfr_interp_code((inst *)p, HCFR_CALIB_CALC);

	/* Multiply by target values */
	icmMul3x3_2(p->crt, xmat, itmat);

	/* LCD */

	/* Red test patch, sensor then reference */
	tmat[0][0] = 3994.356609;
	tmat[1][0] = 1159.679928;
	tmat[2][0] = 818.430397;

	xmat[0][0] = 51.875052;
	xmat[1][0] = 30.640815;
	xmat[2][0] = 4.712397;

	/* Green test patch, sensor then reference */
	tmat[0][1] = 1445.920285;
	tmat[1][1] = 3382.116329;
	tmat[2][1] = 1764.558523;

	xmat[0][1] = 37.482638;
	xmat[1][1] = 64.670821;
	xmat[2][1] = 14.554874;

	/* Blue test patch, sensor then reference */
	tmat[0][2] = 829.727493;
	tmat[1][2] = 1795.182031;
	tmat[2][2] = 3820.123872;

	xmat[0][2] = 25.098392;
	xmat[1][2] = 23.719352;
	xmat[2][2] = 108.134087;

	/* Compute the inverse */
	if (icmInverse3x3(itmat, tmat))
		return hcfr_interp_code((inst *)p, HCFR_CALIB_CALC);

	/* Multiply by target values */
	icmMul3x3_2(p->lcd, xmat, itmat);

	return inst_ok;
}

/* ==================================================================== */

/* Establish communications with a HCFR */
/* Return HCFR_COMS_FAIL on failure to establish communications */
static inst_code
hcfr_init_coms(inst *pp, int port, baud_rate br, flow_control fc, double tout) {
	hcfr *p = (hcfr *) pp;
	int rsize;
	long etime;
	int bi, i, rv;
	inst_code ev = inst_ok;
	icomuflags usbflags = icomuf_no_open_clear | icomuf_detach;

#if defined(__APPLE__) && defined(__i386__)
	/* Except on Intel OS X 10.4/5 for some reasone. */
	/* It would be good if the HCFR had a better USB implementation... */
	usbflags &= ~icomuf_no_open_clear;
#endif

	if (p->debug) {
		p->icom->debug = p->debug;	/* Turn on debugging */
		fprintf(stderr,"hcfr: About to init coms\n");
	}

	if (p->icom->is_usb_portno(p->icom, port) == instUnknown) {
		if (p->debug) fprintf(stderr,"hcfr: init_coms called to wrong device!\n");

		return HCFR_UNKNOWN_MODEL;
	}

	if (p->debug) fprintf(stderr,"hcfr: About to init USB\n");

	/* Set config, interface, "Serial" write & read end points */
	/* Note if we clear halt the interface hangs */
	p->icom->set_usb_port(p->icom, port, 1, 0x03, 0x83, usbflags, 0); 

	if ((ev = hcfr_break(p)) != inst_ok) {
		if (p->debug) fprintf(stderr,"hcfr: Error doing break\n");
		return ev;
	}
	p->gotcoms = 1;

	return inst_ok;
}

/* Initialise the HCFR */
/* return non-zero on an error, with dtp error code */
static inst_code
hcfr_init_inst(inst *pp) {
	hcfr *p = (hcfr *)pp;
	static char buf[MAX_MES_SIZE];
	inst_code ev = inst_ok;

	if (p->debug) fprintf(stderr,"hcfr: About to init instrument\n");

	if (p->gotcoms == 0)
		return inst_internal_error;		/* Must establish coms before calling init */

	hcfr_flush(p);

	if ((ev = hcfr_get_check_version(p, &p->maj, &p->min)) != inst_ok) {
		if (p->debug) fprintf(stderr,"hcfr: Error with getting or version of firmware\n");
		return ev;
	}

	if ((ev = hcfr_comp_matrix(p)) != inst_ok) {
		return ev;
	}

	p->trig = inst_opt_trig_keyb;

	if (ev == inst_ok) {
		p->inited = 1;
		if (p->debug) fprintf(stderr,"hcfr: instrument inited OK\n");
	}

	return ev;
}

/* For an xy instrument, release the paper */
/* Return the inst error code */
static inst_code
hcfr_xy_sheet_release(
struct _inst *p) {
	/* This is not supported by this device */
	return inst_unsupported;
}

/* For an xy instrument, hold the paper */
/* Return the inst error code */
static inst_code 
hcfr_xy_sheet_hold(
struct _inst *p) {
	/* This is not supported by this device */
	return inst_unsupported;
}

/* For an xy instrument, allow the user to locate a point */
/* Return the inst error code */
static inst_code
hcfr_xy_locate_start(
struct _inst *p) {
	/* This is not supported by this device */
	return inst_unsupported;
}

/* For an xy instrument, read back the location */
/* Return the inst error code */
static inst_code
hcfr_xy_get_location(
struct _inst *p,
double *x, double *y) {
	/* This is not supported by this device */
	return inst_unsupported;
}

/* For an xy instrument, ends allowing the user to locate a point */
/* Return the inst error code */
static inst_code
hcfr_xy_locate_end(
struct _inst *p) {
	/* This is not supported by this device */
	return inst_unsupported;
}

/* For an xy instrument, try and clear the table after an abort */
/* Return the inst error code */
static inst_code
hcfr_xy_clear(
struct _inst *p) {
	/* This is not supported by this device */
	return inst_unsupported;
}

/* Read a sheet full of patches using xy mode */
/* Return the inst error code */
static inst_code
hcfr_read_xy(
inst *p,
int pis,				/* Passes in strip (letters in sheet) */
int sip,				/* Steps in pass (numbers in sheet) */
int npatch,				/* Total patches in strip (skip in last pass) */
char *pname,			/* Starting pass name (' A' to 'ZZ') */
char *sname,			/* Starting step name (' 1' to '99') */
double ox, double oy,	/* Origin of first patch */
double ax, double ay,	/* pass increment */
double aax, double aay,	/* pass offset for odd patches */
double px, double py,	/* step/patch increment */
ipatch *vals) { 		/* Pointer to array of values */
	/* This is not supported by this device */
	return inst_unsupported;
}


/* Read a set of strips */
/* Return the dtp error code */
static inst_code
hcfr_read_strip(
inst *pp,
char *name,			/* Strip name (7 chars) */
int npatch,			/* Number of patches in the pass */
char *pname,		/* Pass name (3 chars) */
int sguide,			/* Guide number */
double pwid,		/* Patch length in mm (DTP41) */
double gwid,		/* Gap length in mm (DTP41) */
double twid,		/* Trailer length in mm (DTP41T) */
ipatch *vals) {		/* Pointer to array of instrument patch values */
//	hcfr *p = (hcfr *)pp;

	/* This is not supported by HCFR */
	return inst_unsupported;
}

/* Read a single sample */
/* Return the dtp error code */
static inst_code
hcfr_read_sample(
inst *pp,
char *name,			/* Strip name (7 chars) */
ipatch *val) {		/* Pointer to instrument patch value */
	hcfr *p = (hcfr *)pp;
	inst_code ev;
	double rgb[3];
	int user_trig = 0;

	if (p->trig == inst_opt_trig_keyb) {
		int se;
		if ((se = icoms_poll_user(p->icom, 1)) != ICOM_TRIG) {
			/* Abort, term or command */
			return hcfr_interp_code((inst *)p, icoms2hcfr_err(se));
		}
		user_trig = 1;
		if (p->trig_return)
			printf("\n");
	}

	if ((ev = hcfr_get_rgb(p, rgb)) != inst_ok)
		return ev;

	if (p->cal_mode == 0) {	/* CRT */
		icmMulBy3x3(val->aXYZ, p->crt, rgb);

	} else if (p->cal_mode == 1) {	/* LCD */
		icmMulBy3x3(val->aXYZ, p->lcd, rgb);

	} else {				/* Raw */
		val->aXYZ[0] = rgb[0];
		val->aXYZ[1] = rgb[1];
		val->aXYZ[2] = rgb[2];
	}
	val->aXYZ_v = 1;		/* These are absolute XYZ readings */
	val->XYZ_v = 0;
	val->Lab_v = 0;
	val->sp.spec_n = 0;
	val->duration = 0.0;

	if (user_trig)
		return inst_user_trig;
	return inst_ok;
}

/* Determine if a calibration is needed. Returns inst_calt_none if not, */
/* inst_calt_unknown if it is unknown, or inst_calt_XXX if needs calibration, */
/* and the first type of calibration needed. */
inst_cal_type hcfr_needs_calibration(inst *pp) {
	hcfr *p = (hcfr *)pp;
	return inst_unsupported;
}

/* Request an instrument calibration. */
/* This is use if the user decides they want to do a calibration */
/* in anticipation of a calibration (needs_calibration()) to avoid */
/* requiring one during measurement, or in response to measuring */
/* returning inst_needs_cal. Initially use an inst_cal_cond of inst_calc_none, */
/* and then be prepared to setup the right conditions, or ask the */
/* user to do so, each time the error inst_cal_setup is returned. */
inst_code hcfr_calibrate(
inst *pp,
inst_cal_type calt,		/* Calibration type. inst_calt_all for all neeeded */
inst_cal_cond *calc,	/* Current condition/desired condition */
char id[CALIDLEN]		/* Condition identifier (ie. white reference ID) */
) {
	hcfr *p = (hcfr *)pp;
	return inst_unsupported;
}

/* Insert a compensation filter in the instrument readings */
/* This is typically needed if an adapter is being used, that alters */
/* the spectrum of the light reaching the instrument */
/* To remove the filter, pass NULL for the filter filename */
inst_code hcfr_comp_filter(
struct _inst *p,
char *filtername
) {
	return inst_unsupported;
}

/* Error codes interpretation */
static char *
hcfr_interp_error(inst *pp, int ec) {
//	hcfr *p = (hcfr *)pp;
	ec &= inst_imask;
	switch (ec) {
		case HCFR_INTERNAL_ERROR:
			return "Internal software error";
		case HCFR_COMS_FAIL:
			return "Communications failure";
		case HCFR_UNKNOWN_MODEL:
			return "Not a HCFR or DTP52";
		case HCFR_DATA_PARSE_ERROR:
			return "Data from DTP didn't parse as expected";
		case HCFR_USER_ABORT:
			return "User hit Abort key";
		case HCFR_USER_TERM:
			return "User hit Terminate key";
		case HCFR_USER_TRIG:
			return "User hit Trigger key";
		case HCFR_USER_CMND:
			return "User hit a Command key";

		case HCFR_OK:
			return "No device error";

		case HCFR_BAD_READING:
			return "Invalid reading";

		case HCFR_BAD_FIRMWARE:
			return "Bad firmware version";

		case HCFR_CALIB_CALC:
			return "Error computing calibration matrix";

		default:
			return "Unknown error code";
	}
}


/* Convert a machine specific error code into an abstract dtp code */
static inst_code 
hcfr_interp_code(inst *pp, int ec) {
	hcfr *p = (hcfr *)pp;

	ec &= inst_imask;
	switch (ec) {

		case HCFR_OK:
			return inst_ok;

		case HCFR_CALIB_CALC:
			return inst_internal_error | ec;

		case HCFR_COMS_FAIL:
			return inst_coms_fail | ec;

		case HCFR_UNKNOWN_MODEL:
			return inst_unknown_model | ec;

		case HCFR_DATA_PARSE_ERROR:
			return inst_protocol_error | ec;

		case HCFR_USER_ABORT:
			return inst_user_abort | ec;
		case HCFR_USER_TERM:
			return inst_user_term | ec;
		case HCFR_USER_TRIG:
			return inst_user_trig | ec;
		case HCFR_USER_CMND:
			return inst_user_cmnd | ec;

		case HCFR_BAD_READING:
			return inst_misread | ec;

//		case HCFR_NEEDS_OFFSET_CAL:
//			return inst_needs_cal | ec;

		case HCFR_BAD_FIRMWARE:
			return inst_hardware_fail | ec;
	}
	return inst_other_error | ec;
}

/* Return the last communication error code */
static int
hcfr_last_comerr(inst *pp) {
	hcfr *p = (hcfr *)pp;
	return p->icom->lerr;
}

/* Destroy ourselves */
static void
hcfr_del(inst *pp) {
	hcfr *p = (hcfr *)pp;
	if (p->icom != NULL)
		p->icom->del(p->icom);
	free(p);
}

/* Return the instrument capabilities */
inst_capability hcfr_capabilities(inst *pp) {
	hcfr *p = (hcfr *)pp;
	inst_capability rv;

	rv = inst_emis_spot
	   | inst_emis_disp
	   | inst_colorimeter
	   | inst_emis_disp_crt
	   | inst_emis_disp_lcd
	   ;
	return rv;
}

/* Return the instrument capabilities 2 */
inst2_capability hcfr_capabilities2(inst *pp) {
	hcfr *p = (hcfr *)pp;
	inst2_capability rv;

	rv = inst2_prog_trig
	   | inst2_keyb_trig
	   ;

	return rv;
}

/* Set device measurement mode */
inst_code hcfr_set_mode(inst *pp, inst_mode m)
{
	inst_mode mm;		/* Measurement mode */

	/* The measurement mode portion of the mode */
	mm = m & inst_mode_measurement_mask;

	/* only display emission mode supported */
	if (mm != inst_mode_emis_disp
	 && mm != inst_mode_emis_spot) {
		return inst_unsupported;
	}

	/* Spectral mode is not supported */
	if (m & inst_mode_spectral)
		return inst_unsupported;

	return inst_ok;
}

/* 
 * set or reset an optional mode
 * We assume that the instrument has been initialised.
 */
static inst_code
hcfr_set_opt_mode(inst *pp, inst_opt_mode m, ...)
{
	hcfr *p = (hcfr *)pp;
	inst_code ev = inst_ok;

#ifdef MEASURE_RAW
	p->cal_mode = 2;
	return inst_ok;
#endif
	if (m == inst_opt_disp_crt) {
		p->cal_mode = 0;
		return inst_ok;
	} else if (m == inst_opt_disp_lcd) {
		p->cal_mode = 1;
		return inst_ok;
	}

	/* Record the trigger mode */
	if (m == inst_opt_trig_prog
	 || m == inst_opt_trig_keyb) {
		p->trig = m;
		return inst_ok;
	}
	if (m == inst_opt_trig_return) {
		p->trig_return = 1;
		return inst_ok;
	} else if (m == inst_opt_trig_no_return) {
		p->trig_return = 0;
		return inst_ok;
	}

	return inst_unsupported;
}

/* Constructor */
extern hcfr *new_hcfr(icoms *icom, int debug, int verb)
{
	hcfr *p;
	if ((p = (hcfr *)calloc(sizeof(hcfr),1)) == NULL)
		error("hcfr: malloc failed!");

	if (icom == NULL)
		p->icom = new_icoms();
	else
		p->icom = icom;

	p->debug = debug;
	p->verb = verb;

	p->init_coms        = hcfr_init_coms;
	p->init_inst        = hcfr_init_inst;
	p->capabilities     = hcfr_capabilities;
	p->capabilities2    = hcfr_capabilities2;
	p->set_mode         = hcfr_set_mode;
	p->set_opt_mode     = hcfr_set_opt_mode;
	p->xy_sheet_release = hcfr_xy_sheet_release;
	p->xy_sheet_hold    = hcfr_xy_sheet_hold;
	p->xy_locate_start  = hcfr_xy_locate_start;
	p->xy_get_location  = hcfr_xy_get_location;
	p->xy_locate_end    = hcfr_xy_locate_end;
	p->xy_clear         = hcfr_xy_clear;
	p->read_xy          = hcfr_read_xy;
	p->read_strip       = hcfr_read_strip;
	p->read_sample      = hcfr_read_sample;
	p->needs_calibration = hcfr_needs_calibration;
	p->calibrate        = hcfr_calibrate;
	p->comp_filter    	= hcfr_comp_filter;
	p->interp_error     = hcfr_interp_error;
	p->inst_interp_error = NULL;				/* virtual constructor will do this */
	p->last_comerr      = hcfr_last_comerr;
	p->del              = hcfr_del;

	p->itype = instHCFR;

	return p;
}


