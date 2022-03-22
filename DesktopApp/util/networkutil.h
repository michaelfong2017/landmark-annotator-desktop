#ifndef NETWORKUTIL_H
#define NETWORKUTIL_H

#include <QtNetwork>

static void uploadRGBImageArrayAndDepthToRGBImageArray(QNetworkAccessManager &manager, QUrl qUrl, QString imageId, qint8 imageType, QImage colorImage, QImage depthToColorImage) {
	QNetworkRequest request(qUrl);

	QHttpMultiPart* multipart = new QHttpMultiPart(QHttpMultiPart::FormDataType);

	// image_id
	QHttpPart imageIdPart;
	imageIdPart.setHeader(QNetworkRequest::ContentDispositionHeader, QVariant("form-data; name=\"image_id\""));
	imageIdPart.setBody(imageId.toUtf8());
	multipart->append(imageIdPart);

	// image_type
	QHttpPart imageTypePart;
	imageTypePart.setHeader(QNetworkRequest::ContentDispositionHeader, QVariant("form-data; name=\"image_type\""));
	imageTypePart.setBody(QByteArray::number(imageType));
	multipart->append(imageTypePart);

	// rgb_image_array
	QHttpPart RGBImageArray;
	RGBImageArray.setHeader(QNetworkRequest::ContentTypeHeader, QVariant("image/png"));
	RGBImageArray.setHeader(QNetworkRequest::ContentDispositionHeader, QVariant("form-data; name=\"rgb_image_array\""));
	QByteArray byteArray;
	QBuffer buffer(&byteArray);
	colorImage.save(&buffer, "PNG");
	RGBImageArray.setBody(byteArray);
	multipart->append(RGBImageArray);

	// depth_to_rgb_image_array
	QHttpPart DepthToRGBImageArray;
	DepthToRGBImageArray.setHeader(QNetworkRequest::ContentTypeHeader, QVariant("image/png"));
	DepthToRGBImageArray.setHeader(QNetworkRequest::ContentDispositionHeader, QVariant("form-data; name=\"depth_to_rgb_image_array\""));
	QByteArray byteArrayDepthToRGB;
	QBuffer bufferDepthToRGB(&byteArrayDepthToRGB);
	colorImage.save(&bufferDepthToRGB, "PNG");
	DepthToRGBImageArray.setBody(byteArrayDepthToRGB);
	multipart->append(DepthToRGBImageArray);

	manager.post(request, multipart);
}

#endif
