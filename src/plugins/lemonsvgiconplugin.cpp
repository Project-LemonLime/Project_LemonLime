/*
 * SPDX-FileCopyrightText: 2026 Project LemonLime
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 * Design reference: KDE KIconThemes, KIconColors::stylesheet() and
 * KIconLoaderPrivate::processSvg(), revision f4f5e4e01c22fba7eff981887c68724b456de2c2.
 * Independently implemented here; no KIconThemes source is incorporated.
 * See README.md in this directory for reference URLs and licensing details.
 *
 */

#include <QCryptographicHash>
#include <QFile>
#include <QGuiApplication>
#include <QIconEngine>
#include <QIconEnginePlugin>
#include <QPainter>
#include <QPalette>
#include <QPixmapCache>
#include <QSvgRenderer>
#include <QXmlStreamReader>
#include <QXmlStreamWriter>

namespace {
	QString iconStyleSheet(const QPalette &palette, QPalette::ColorGroup group, QIcon::Mode mode) {
		const QColor text =
		    palette.color(group, mode == QIcon::Selected ? QPalette::HighlightedText : QPalette::ButtonText);
		const bool dark = palette.color(group, QPalette::Button).lightness() <
		                  palette.color(group, QPalette::ButtonText).lightness();
		const QColor negative = mode == QIcon::Disabled || mode == QIcon::Selected ? text
		                        : dark                                             ? QColor(255, 128, 139)
		                                                                           : QColor(218, 68, 83);
		// Keep application-specific semantic colors here, separate from SVG geometry.
		return QStringLiteral(".ColorScheme-Text { color: %1; }\n"
		                      ".ColorScheme-NegativeText { color: %2; }\n")
		    .arg(text.name(), negative.name());
	}

	QByteArray applyStyleSheet(const QByteArray &svg, const QString &styleSheet) {
		QXmlStreamReader reader(svg);
		QByteArray result;
		QXmlStreamWriter writer(&result);
		bool themed = false;
		while (! reader.atEnd()) {
			reader.readNext();
			if (reader.hasError())
				return {};
			writer.writeCurrentToken(reader);
			if (reader.isStartElement() && reader.name() == QLatin1String("style") &&
			    (reader.namespaceUri().isEmpty() ||
			     reader.namespaceUri() == QLatin1String("http://www.w3.org/2000/svg")) &&
			    reader.attributes().value(QLatin1String("id")) == QLatin1String("current-color-scheme")) {
				writer.writeCharacters(styleSheet);
				reader.skipCurrentElement();
				writer.writeEndElement();
				themed = true;
			}
		}
		if (reader.hasError() || writer.hasError())
			return {};
		return themed ? result : svg;
	}

	class LemonSvgIconEngine : public QIconEngine {
	  public:
		explicit LemonSvgIconEngine(const QString &fileName) {
			addFile(fileName, {}, QIcon::Normal, QIcon::Off);
		}
		QIconEngine *clone() const override { return new LemonSvgIconEngine(*this); }
		QString key() const override { return QStringLiteral("llsvg"); }
		bool isNull() override { return svg.isEmpty(); }

		void addFile(const QString &name, const QSize &, QIcon::Mode, QIcon::State) override {
			fileName = name;
			svg.clear();
			sourceId.clear();
			lastStyleSheet.clear();
			themedSvg.clear();
			defaultSize = {};
			if (name.isEmpty())
				return;
			QFile file(name);
			if (! file.open(QIODevice::ReadOnly))
				return;
			const QByteArray data = file.readAll();
			QSvgRenderer renderer(data);
			if (! renderer.isValid())
				return;
			svg = data;
			defaultSize = renderer.defaultSize();
			sourceId =
			    QString::fromLatin1(QCryptographicHash::hash(data, QCryptographicHash::Sha256).toHex());
		}

		QSize actualSize(const QSize &size, QIcon::Mode, QIcon::State) override {
			return defaultSize.isEmpty() ? QSize() : defaultSize.scaled(size, Qt::KeepAspectRatio);
		}

		QPixmap pixmap(const QSize &size, QIcon::Mode mode, QIcon::State state) override {
			return scaledPixmap(size, mode, state, 1.0);
		}

		QPixmap scaledPixmap(const QSize &size, QIcon::Mode mode, QIcon::State state, qreal scale) override {
			if (isNull() || size.isEmpty() || scale <= 0)
				return {};
			const QPalette palette = QGuiApplication::palette();
			const auto group = mode == QIcon::Disabled ? QPalette::Disabled
			                   : QGuiApplication::applicationState() == Qt::ApplicationActive
			                       ? QPalette::Active
			                       : QPalette::Inactive;
			const QString cacheKey = QStringLiteral("lemon-svg:%1:%2:%3:%4:%5x%6:%7:%8")
			                             .arg(sourceId)
			                             .arg(palette.cacheKey())
			                             .arg(int(group))
			                             .arg(int(mode))
			                             .arg(size.width())
			                             .arg(size.height())
			                             .arg(scale, 0, 'g', 17)
			                             .arg(int(state));
			QPixmap result;
			if (QPixmapCache::find(cacheKey, &result))
				return result;

			const QString styleSheet = iconStyleSheet(palette, group, mode);
			if (styleSheet != lastStyleSheet) {
				themedSvg = applyStyleSheet(svg, styleSheet);
				lastStyleSheet = styleSheet;
			}
			QSvgRenderer renderer(themedSvg);
			if (! renderer.isValid())
				return {};
			renderer.setAspectRatioMode(Qt::KeepAspectRatio);
			result = QPixmap(size * scale);
			result.setDevicePixelRatio(scale);
			result.fill(Qt::transparent);
			{
				QPainter painter(&result);
				renderer.render(&painter, QRectF(QPointF(), QSizeF(size)));
			}
			QPixmapCache::insert(cacheKey, result);
			return result;
		}

		void paint(QPainter *painter, const QRect &rect, QIcon::Mode mode, QIcon::State state) override {
			painter->drawPixmap(
			    rect, scaledPixmap(rect.size(), mode, state, painter->device()->devicePixelRatioF()));
		}

	  private:
		QString fileName;
		QByteArray svg;
		QString sourceId;
		QSize defaultSize;
		QString lastStyleSheet;
		QByteArray themedSvg;
	};
} // namespace

class LemonSvgIconPlugin : public QIconEnginePlugin {
	Q_OBJECT
	Q_PLUGIN_METADATA(IID QIconEngineFactoryInterface_iid FILE "lemonsvgiconplugin.json")

  public:
	QIconEngine *create(const QString &fileName = QString()) override {
		return new LemonSvgIconEngine(fileName);
	}
};

#include "lemonsvgiconplugin.moc"
