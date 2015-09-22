#ifndef TEXTTHREAD
#define TEXTTHREAD

#include <ctime>
#include <stdint.h>

#include <QThread>
#include <QtCore>
#include <QPixmap>
#include <QImage>

#define PACKET_SIZE 164
#define PACKET_SIZE_UINT16 (PACKET_SIZE/2)
#define PACKETS_PER_FRAME_IMAGE 60
#define PACKETS_PER_FRAME_TELEMETRY 3
#define PACKETS_PER_FRAME (PACKETS_PER_FRAME_IMAGE+PACKETS_PER_FRAME_TELEMETRY)
#define FRAME_SIZE_IMAGE_UINT16 (PACKET_SIZE_UINT16*PACKETS_PER_FRAME_IMAGE)

typedef struct RGBColor_t { unsigned char red, green, blue; } RGBColor;

typedef struct TELEMETRY_INFO_t {
  int packetNumber;
  float fpa_temp;
  float housing_temp;
} TELEMETRY_INFO;

class LeptonThread : public QThread
{
  Q_OBJECT;

private:

  QImage myImage;

  uint8_t image[PACKET_SIZE*PACKETS_PER_FRAME_IMAGE];
  TELEMETRY_INFO telemetry;
  uint16_t *frameBuffer;
  RGBColor HSL2RGB(double h, double sl, double l);

public:
  LeptonThread();
  ~LeptonThread();

  void run();

public slots:
  void performFFC();
  void parseTelemetryPacket(uint8_t *packet);

signals:
  void updateText(QString);
  void updateImage(QImage);

};

#endif
