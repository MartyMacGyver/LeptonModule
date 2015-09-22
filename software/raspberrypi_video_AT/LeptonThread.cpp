#include "LeptonThread.h"

#include "Palettes.h"
#include "SPI.h"
#include "Lepton_I2C.h"
#include "gpio.h"

#define PACKET_SIZE 164
#define PACKET_SIZE_UINT16 (PACKET_SIZE/2)
#define PACKETS_PER_FRAME_IMAGE 60
#define PACKETS_PER_FRAME_TELEMETRY 3
#define PACKETS_PER_FRAME (PACKETS_PER_FRAME_IMAGE+PACKETS_PER_FRAME_TELEMETRY)
#define FRAME_SIZE_IMAGE_UINT16 (PACKET_SIZE_UINT16*PACKETS_PER_FRAME_IMAGE)
#define FPS 27;

#define VALUE_MAX 128
#define LOW 0


LeptonThread::LeptonThread() : QThread()
{
}

LeptonThread::~LeptonThread() {
}

void LeptonThread::run()
{
	int resets = 0;
	int packetNumber = 0;
	uint8_t packet_buff[PACKET_SIZE];
	int first_frame = 1;
	FILE *fp;

	//create the initial image
	myImage = QImage(80, 60, QImage::Format_RGB888);

	// red dot issue fix => sync camera by toggling CS pin
	gpio_state(8, 1);
	usleep(200); // wait > 185ms to resync
	gpio_state(8, 0);

	// disable auto-FFC
	lepton_disable_auto_ffc();

	// enable telemetry (footer, i.e. packets 61-63)
	lepton_enable_telemetry(1);

	//open spi port
	SpiOpenPort(0);

	while(true) {

		//read data packets from lepton over SPI
		resets = 0;
		packetNumber = 0;
		for(int j=0;j<PACKETS_PER_FRAME;j++) {
			//if it's a drop packet, reset j to 0, set to -1 so he'll be at 0 again loop
			if (j < PACKETS_PER_FRAME_IMAGE) {
				read(spi_cs0_fd, image+sizeof(uint8_t)*PACKET_SIZE*j, sizeof(uint8_t)*PACKET_SIZE);
				packetNumber = image[j*PACKET_SIZE+1];
			} else {
				read(spi_cs0_fd, packet_buff, sizeof(uint8_t)*PACKET_SIZE);
				if (j == PACKETS_PER_FRAME_IMAGE) {
					parseTelemetryPacket(packet_buff);
				}
				packetNumber = packet_buff[1];
			}
			if(packetNumber != j) {
				j = -1;
				resets += 1;
				usleep(1000);
				//Note: we've selected 750 resets as an arbitrary limit, since there should never be 750 "null" packets between two valid transmissions at the current poll rate
				//By polling faster, developers may easily exceed this count, and the down period between frames may then be flagged as a loss of sync
				if(resets == 750) {
					SpiClosePort(0);
					usleep(750000);
					SpiOpenPort(0);
				}
			}
		}
		if(resets >= 30) {
			qDebug() << "done reading, resets: " << resets;
		}

		frameBuffer = (uint16_t *)image;
		int row, column;
		uint16_t value;
		uint16_t minValue = 65535;
		uint16_t maxValue = 0;

		if (first_frame) {
			fp=fopen("imagedata.txt", "w+");
		}

		for(int i=0;i<FRAME_SIZE_IMAGE_UINT16;i++) {
			//skip the first 2 uint16_t's of every packet, they're 4 header bytes
			if(i % PACKET_SIZE_UINT16 < 2) {
				continue;
			}

			//flip the MSB and LSB at the last second
			int temp = image[i*2];
			image[i*2] = image[i*2+1];
			image[i*2+1] = temp;

			if (first_frame) {
				fprintf(fp, "%lu ", (unsigned long)frameBuffer[i]);
			}
			
			value = frameBuffer[i];
			if(value > maxValue) {
				maxValue = value;
			}
			if(value < minValue) {
				minValue = value;
			}
			column = i % PACKET_SIZE_UINT16 - 2;
			row = i / PACKET_SIZE_UINT16 ;
		}

		if (first_frame) {
			first_frame = 0;
			fclose(fp);
		}

		double diff = maxValue - minValue;
		double value_d = 0;
		//float scale = 255/diff;
		double hue = 0;
		QRgb color;
		RGBColor rgb;
		for(int i=0;i<FRAME_SIZE_IMAGE_UINT16;i++) {
			if(i % PACKET_SIZE_UINT16 < 2) {
				continue;
			}
			/*value = (frameBuffer[i] - minValue) * scale;
			const int *colormap = colormap_rainbow;
			color = qRgb(colormap[3*value], colormap[3*value+1], colormap[3*value+2]);
			*/
			value_d = (frameBuffer[i] - minValue);
			hue = 1.0f - (value_d / diff);
			//hue *= 0.65f;
			rgb = HSL2RGB(hue, 1.0f, 0.5f);
			color = qRgb(rgb.red, rgb.green, rgb.blue);
			column = (i % PACKET_SIZE_UINT16 ) - 2;
			row = i / PACKET_SIZE_UINT16;
			myImage.setPixel(column, 59-row, color);
		}

		//lets emit the signal for update
		emit updateImage(myImage);

	}
	
	//finally, close SPI port just bcuz
	SpiClosePort(0);
}

void LeptonThread::parseTelemetryPacket(uint8_t *packet) {
	telemetry.packetNumber = packet[1];
	// read 16bit numbers first
	uint16_t ft = packet[52] << 8 | packet[53];
	uint16_t ht = packet[56] << 8 | packet[57];
	// now convert from kelvinx100 to celcius
	telemetry.fpa_temp = (float)( ( ( ft / 100 ) + ( ( ft % 100 ) * .01 ) ) - 273.15 );
	telemetry.housing_temp = (float)( ( ( ht / 100 ) + ( ( ht % 100 ) * .01 ) ) - 273.15 );
}

void LeptonThread::performFFC() {
	//perform FFC
	lepton_perform_ffc();

	//qDebug() << "Enabling Telemetry: " << lepton_enable_telemetry(0);

	LEP_SYS_FFC_SHUTTER_MODE_OBJ_T smo = lepton_get_ffc_shutter_mode();
	qDebug() << "shutterMode: " << smo.shutterMode << "\ntempLockoutState: " << smo.tempLockoutState << "\nvideoFreezeDuringFFC: " << smo.videoFreezeDuringFFC << "\nffcDesired: " << smo.ffcDesired << "\nelapsedTimeSinceLastFfc: " << smo.elapsedTimeSinceLastFfc << "\ndesiredFfcPeriod: " << smo.desiredFfcPeriod << "\nexplicitCmdToOpen: " << (int)smo.explicitCmdToOpen << "\ndesiredFfcTempDelta: " << smo.desiredFfcTempDelta << "\nimminentDelay: " << smo.imminentDelay << "\n";
	
	//LEP_SYS_SHUTTER_POSITION_E spos = lepton_toggle_shutter();
	//qDebug() << "shutter position: " << spos << "\n";

	// report temperatures
	qDebug() << "AUX Temp: " << lepton_get_aux_temperature();
	qDebug() << "FPA Temp: " << lepton_get_fpa_temperature();
}

// Given H,S,L in range of 0-1
// Returns a Color (RGB struct) in range of 0-255
RGBColor LeptonThread::HSL2RGB(double h, double sl, double l) {
	double v;
	double r,g,b;

	r = l;   // default to gray
	g = l;
	b = l;
	v = (l <= 0.5) ? (l * (1.0 + sl)) : (l + sl - l * sl);
	if (v > 0)
	{
		double m;
		double sv;
		int sextant;
		double fract, vsf, mid1, mid2;

		m = l + l - v;
		sv = (v - m ) / v;
		h *= 6.0;
		sextant = (int)h;
		fract = h - sextant;
		vsf = v * sv * fract;
		mid1 = m + vsf;
		mid2 = v - vsf;
		switch (sextant)
		{
		case 0:
			r = v;
			g = mid1;
			b = m;
			break;
		case 1:
			r = mid2;
			g = v;
			b = m;
			break;
		case 2:
			r = m;
			g = v;
			b = mid1;
			break;
		case 3:
			r = m;
			g = mid2;
			b = v;
			break;
		case 4:
			r = mid1;
			g = m;
			b = v;
			break;
		case 5:
			r = v;
			g = m;
			b = mid2;
			break;
		}
	}
	RGBColor rgb;
	rgb.red = (r * 255.0f);
	rgb.green = (g * 255.0f);
	rgb.blue = (b * 255.0f);
	return rgb;
}