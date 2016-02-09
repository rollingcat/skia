
/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#include "SkDrawCommand.h"
#include "SkBlurMaskFilter.h"
#include "SkColorFilter.h"
#include "SkDashPathEffect.h"
#include "SkImageFilter.h"
#include "SkMaskFilter.h"
#include "SkObjectParser.h"
#include "SkPaintDefaults.h"
#include "SkPathEffect.h"
#include "SkPicture.h"
#include "SkTextBlob.h"
#include "SkTextBlobRunIterator.h"
#include "SkTHash.h"
#include "SkTypeface.h"
#include "SkValidatingReadBuffer.h"
#include "SkWriteBuffer.h"

#define SKDEBUGCANVAS_SEND_BINARIES               false

#define SKDEBUGCANVAS_ATTRIBUTE_COMMAND           "command"
#define SKDEBUGCANVAS_ATTRIBUTE_MATRIX            "matrix"
#define SKDEBUGCANVAS_ATTRIBUTE_COORDS            "coords"
#define SKDEBUGCANVAS_ATTRIBUTE_BOUNDS            "bounds"
#define SKDEBUGCANVAS_ATTRIBUTE_PAINT             "paint"
#define SKDEBUGCANVAS_ATTRIBUTE_OUTER             "outer"
#define SKDEBUGCANVAS_ATTRIBUTE_INNER             "inner"
#define SKDEBUGCANVAS_ATTRIBUTE_MODE              "mode"
#define SKDEBUGCANVAS_ATTRIBUTE_POINTS            "points"
#define SKDEBUGCANVAS_ATTRIBUTE_PATH              "path"
#define SKDEBUGCANVAS_ATTRIBUTE_TEXT              "text"
#define SKDEBUGCANVAS_ATTRIBUTE_COLOR             "color"
#define SKDEBUGCANVAS_ATTRIBUTE_ALPHA             "alpha"
#define SKDEBUGCANVAS_ATTRIBUTE_STYLE             "style"
#define SKDEBUGCANVAS_ATTRIBUTE_STROKEWIDTH       "strokeWidth"
#define SKDEBUGCANVAS_ATTRIBUTE_STROKEMITER       "strokeMiter"
#define SKDEBUGCANVAS_ATTRIBUTE_CAP               "cap"
#define SKDEBUGCANVAS_ATTRIBUTE_ANTIALIAS         "antiAlias"
#define SKDEBUGCANVAS_ATTRIBUTE_REGION            "region"
#define SKDEBUGCANVAS_ATTRIBUTE_REGIONOP          "op"
#define SKDEBUGCANVAS_ATTRIBUTE_EDGESTYLE         "edgeStyle"
#define SKDEBUGCANVAS_ATTRIBUTE_DEVICEREGION      "deviceRegion"
#define SKDEBUGCANVAS_ATTRIBUTE_BLUR              "blur"
#define SKDEBUGCANVAS_ATTRIBUTE_SIGMA             "sigma"
#define SKDEBUGCANVAS_ATTRIBUTE_QUALITY           "quality"
#define SKDEBUGCANVAS_ATTRIBUTE_TEXTALIGN         "textAlign"
#define SKDEBUGCANVAS_ATTRIBUTE_TEXTSIZE          "textSize"
#define SKDEBUGCANVAS_ATTRIBUTE_TEXTSCALEX        "textScaleX"
#define SKDEBUGCANVAS_ATTRIBUTE_TEXTSKEWX         "textSkewX"
#define SKDEBUGCANVAS_ATTRIBUTE_DASHING           "dashing"
#define SKDEBUGCANVAS_ATTRIBUTE_INTERVALS         "intervals"
#define SKDEBUGCANVAS_ATTRIBUTE_PHASE             "phase"
#define SKDEBUGCANVAS_ATTRIBUTE_FILLTYPE          "fillType"
#define SKDEBUGCANVAS_ATTRIBUTE_VERBS             "verbs"
#define SKDEBUGCANVAS_ATTRIBUTE_NAME              "name"
#define SKDEBUGCANVAS_ATTRIBUTE_BYTES             "bytes"
#define SKDEBUGCANVAS_ATTRIBUTE_SHADER            "shader"
#define SKDEBUGCANVAS_ATTRIBUTE_PATHEFFECT        "pathEffect"
#define SKDEBUGCANVAS_ATTRIBUTE_MASKFILTER        "maskFilter"
#define SKDEBUGCANVAS_ATTRIBUTE_XFERMODE          "xfermode"
#define SKDEBUGCANVAS_ATTRIBUTE_BACKDROP          "backdrop"
#define SKDEBUGCANVAS_ATTRIBUTE_COLORFILTER       "colorfilter"
#define SKDEBUGCANVAS_ATTRIBUTE_IMAGEFILTER       "imagefilter"
#define SKDEBUGCANVAS_ATTRIBUTE_IMAGE             "image"
#define SKDEBUGCANVAS_ATTRIBUTE_BITMAP            "bitmap"
#define SKDEBUGCANVAS_ATTRIBUTE_SRC               "src"
#define SKDEBUGCANVAS_ATTRIBUTE_DST               "dst"
#define SKDEBUGCANVAS_ATTRIBUTE_CENTER            "center"
#define SKDEBUGCANVAS_ATTRIBUTE_STRICT            "strict"
#define SKDEBUGCANVAS_ATTRIBUTE_DESCRIPTION       "description"
#define SKDEBUGCANVAS_ATTRIBUTE_X                 "x"
#define SKDEBUGCANVAS_ATTRIBUTE_Y                 "y"
#define SKDEBUGCANVAS_ATTRIBUTE_RUNS              "runs"
#define SKDEBUGCANVAS_ATTRIBUTE_POSITIONS         "positions"
#define SKDEBUGCANVAS_ATTRIBUTE_GLYPHS            "glyphs"
#define SKDEBUGCANVAS_ATTRIBUTE_FONT              "font"
#define SKDEBUGCANVAS_ATTRIBUTE_TYPEFACE          "typeface"

#define SKDEBUGCANVAS_VERB_MOVE                   "move"
#define SKDEBUGCANVAS_VERB_LINE                   "line"
#define SKDEBUGCANVAS_VERB_QUAD                   "quad"
#define SKDEBUGCANVAS_VERB_CUBIC                  "cubic"
#define SKDEBUGCANVAS_VERB_CONIC                  "conic"
#define SKDEBUGCANVAS_VERB_CLOSE                  "close"

#define SKDEBUGCANVAS_STYLE_FILL                  "fill"
#define SKDEBUGCANVAS_STYLE_STROKE                "stroke"
#define SKDEBUGCANVAS_STYLE_STROKEANDFILL         "strokeAndFill"

#define SKDEBUGCANVAS_POINTMODE_POINTS            "points"
#define SKDEBUGCANVAS_POINTMODE_LINES             "lines"
#define SKDEBUGCANVAS_POINTMODE_POLYGON           "polygon"

#define SKDEBUGCANVAS_REGIONOP_DIFFERENCE         "difference"
#define SKDEBUGCANVAS_REGIONOP_INTERSECT          "intersect"
#define SKDEBUGCANVAS_REGIONOP_UNION              "union"
#define SKDEBUGCANVAS_REGIONOP_XOR                "xor"
#define SKDEBUGCANVAS_REGIONOP_REVERSE_DIFFERENCE "reverseDifference"
#define SKDEBUGCANVAS_REGIONOP_REPLACE            "replace"

#define SKDEBUGCANVAS_BLURSTYLE_NORMAL            "normal"
#define SKDEBUGCANVAS_BLURSTYLE_SOLID             "solid"
#define SKDEBUGCANVAS_BLURSTYLE_OUTER             "outer"
#define SKDEBUGCANVAS_BLURSTYLE_INNER             "inner"

#define SKDEBUGCANVAS_BLURQUALITY_LOW             "low"
#define SKDEBUGCANVAS_BLURQUALITY_HIGH            "high"

#define SKDEBUGCANVAS_ALIGN_LEFT                  "left"
#define SKDEBUGCANVAS_ALIGN_CENTER                "center"
#define SKDEBUGCANVAS_ALIGN_RIGHT                 "right"

#define SKDEBUGCANVAS_FILLTYPE_WINDING            "winding"
#define SKDEBUGCANVAS_FILLTYPE_EVENODD            "evenOdd"
#define SKDEBUGCANVAS_FILLTYPE_INVERSEWINDING     "inverseWinding"
#define SKDEBUGCANVAS_FILLTYPE_INVERSEEVENODD     "inverseEvenOdd"

#define SKDEBUGCANVAS_CAP_BUTT                    "butt"
#define SKDEBUGCANVAS_CAP_ROUND                   "round"
#define SKDEBUGCANVAS_CAP_SQUARE                  "square"

#define SKDEBUGCANVAS_COLORTYPE_ARGB4444          "ARGB4444"
#define SKDEBUGCANVAS_COLORTYPE_RGBA8888          "RGBA8888"
#define SKDEBUGCANVAS_COLORTYPE_BGRA8888          "BGRA8888"
#define SKDEBUGCANVAS_COLORTYPE_565               "565"
#define SKDEBUGCANVAS_COLORTYPE_GRAY8             "Gray8"
#define SKDEBUGCANVAS_COLORTYPE_INDEX8            "Index8"
#define SKDEBUGCANVAS_COLORTYPE_ALPHA8            "Alpha8"

#define SKDEBUGCANVAS_ALPHATYPE_OPAQUE            "opaque"
#define SKDEBUGCANVAS_ALPHATYPE_PREMUL            "premul"
#define SKDEBUGCANVAS_ALPHATYPE_UNPREMUL          "unpremul"

typedef SkDrawCommand* (*FROM_JSON)(Json::Value);

// TODO(chudy): Refactor into non subclass model.

SkDrawCommand::SkDrawCommand(OpType type)
    : fOpType(type)
    , fVisible(true) {
}

SkDrawCommand::~SkDrawCommand() {
    fInfo.deleteAll();
}

const char* SkDrawCommand::GetCommandString(OpType type) {
    switch (type) {
        case kBeginDrawPicture_OpType: return "BeginDrawPicture";
        case kClipPath_OpType: return "ClipPath";
        case kClipRegion_OpType: return "ClipRegion";
        case kClipRect_OpType: return "ClipRect";
        case kClipRRect_OpType: return "ClipRRect";
        case kConcat_OpType: return "Concat";
        case kDrawBitmap_OpType: return "DrawBitmap";
        case kDrawBitmapNine_OpType: return "DrawBitmapNine";
        case kDrawBitmapRect_OpType: return "DrawBitmapRect";
        case kDrawClear_OpType: return "DrawClear";
        case kDrawDRRect_OpType: return "DrawDRRect";
        case kDrawImage_OpType: return "DrawImage";
        case kDrawImageRect_OpType: return "DrawImageRect";
        case kDrawOval_OpType: return "DrawOval";
        case kDrawPaint_OpType: return "DrawPaint";
        case kDrawPatch_OpType: return "DrawPatch";
        case kDrawPath_OpType: return "DrawPath";
        case kDrawPoints_OpType: return "DrawPoints";
        case kDrawPosText_OpType: return "DrawPosText";
        case kDrawPosTextH_OpType: return "DrawPosTextH";
        case kDrawRect_OpType: return "DrawRect";
        case kDrawRRect_OpType: return "DrawRRect";
        case kDrawText_OpType: return "DrawText";
        case kDrawTextBlob_OpType: return "DrawTextBlob";
        case kDrawTextOnPath_OpType: return "DrawTextOnPath";
        case kDrawVertices_OpType: return "DrawVertices";
        case kEndDrawPicture_OpType: return "EndDrawPicture";
        case kRestore_OpType: return "Restore";
        case kSave_OpType: return "Save";
        case kSaveLayer_OpType: return "SaveLayer";
        case kSetMatrix_OpType: return "SetMatrix";
        default:
            SkDebugf("OpType error 0x%08x\n", type);
            SkASSERT(0);
            break;
    }
    SkDEBUGFAIL("DrawType UNUSED\n");
    return nullptr;
}

SkString SkDrawCommand::toString() const {
    return SkString(GetCommandString(fOpType));
}

Json::Value SkDrawCommand::toJSON() const {
    Json::Value result;
    result[SKDEBUGCANVAS_ATTRIBUTE_COMMAND] = this->GetCommandString(fOpType);
    return result;
}

#define INSTALL_FACTORY(name) factories.set(SkString(GetCommandString(k ## name ##_OpType)), \
                                            (FROM_JSON) Sk ## name ## Command::fromJSON)
SkDrawCommand* SkDrawCommand::fromJSON(Json::Value& command) {
    static SkTHashMap<SkString, FROM_JSON> factories;
    static bool initialized = false;
    if (!initialized) {
        initialized = true;
        INSTALL_FACTORY(Restore);
        INSTALL_FACTORY(ClipPath);
        INSTALL_FACTORY(ClipRegion);
        INSTALL_FACTORY(ClipRect);
        INSTALL_FACTORY(ClipRRect);
        INSTALL_FACTORY(Concat);
        INSTALL_FACTORY(DrawBitmap);
        INSTALL_FACTORY(DrawBitmapRect);
        INSTALL_FACTORY(DrawBitmapNine);
        INSTALL_FACTORY(DrawImage);
        INSTALL_FACTORY(DrawImageRect);
        INSTALL_FACTORY(DrawOval);
        INSTALL_FACTORY(DrawPaint);
        INSTALL_FACTORY(DrawPath);
        INSTALL_FACTORY(DrawPoints);
        INSTALL_FACTORY(DrawText);
        INSTALL_FACTORY(DrawPosText);
        INSTALL_FACTORY(DrawTextOnPath);
        INSTALL_FACTORY(DrawTextBlob);

        INSTALL_FACTORY(DrawRect);
        INSTALL_FACTORY(DrawRRect);
        INSTALL_FACTORY(DrawDRRect);
        INSTALL_FACTORY(Save);
        INSTALL_FACTORY(SaveLayer);
        INSTALL_FACTORY(SetMatrix);
    }
    SkString name = SkString(command[SKDEBUGCANVAS_ATTRIBUTE_COMMAND].asCString());
    FROM_JSON* factory = factories.find(name);
    if (factory == nullptr) {
        SkDebugf("no JSON factory for '%s'\n", name.c_str());
        return nullptr;
    }
    return (*factory)(command);
}

SkClearCommand::SkClearCommand(SkColor color) : INHERITED(kDrawClear_OpType) {
    fColor = color;
    fInfo.push(SkObjectParser::CustomTextToString("No Parameters"));
}

void SkClearCommand::execute(SkCanvas* canvas) const {
    canvas->clear(fColor);
}

Json::Value SkClearCommand::toJSON() const {
    Json::Value result = INHERITED::toJSON();
    Json::Value colorValue(Json::arrayValue);
    colorValue.append(Json::Value(SkColorGetA(fColor)));
    colorValue.append(Json::Value(SkColorGetR(fColor)));
    colorValue.append(Json::Value(SkColorGetG(fColor)));
    colorValue.append(Json::Value(SkColorGetB(fColor)));
    result[SKDEBUGCANVAS_ATTRIBUTE_COLOR] = colorValue;;
    return result;
}

 SkClearCommand* SkClearCommand::fromJSON(Json::Value& command) {
    Json::Value color = command[SKDEBUGCANVAS_ATTRIBUTE_COLOR];
    return new SkClearCommand(SkColorSetARGB(color[0].asInt(), color[1].asInt(), color[2].asInt(),
                                             color[3].asInt()));
}

namespace {

void xlate_and_scale_to_bounds(SkCanvas* canvas, const SkRect& bounds) {
    const SkISize& size = canvas->getDeviceSize();

    static const SkScalar kInsetFrac = 0.9f; // Leave a border around object

    canvas->translate(size.fWidth/2.0f, size.fHeight/2.0f);
    if (bounds.width() > bounds.height()) {
        canvas->scale(SkDoubleToScalar((kInsetFrac*size.fWidth)/bounds.width()),
                      SkDoubleToScalar((kInsetFrac*size.fHeight)/bounds.width()));
    } else {
        canvas->scale(SkDoubleToScalar((kInsetFrac*size.fWidth)/bounds.height()),
                      SkDoubleToScalar((kInsetFrac*size.fHeight)/bounds.height()));
    }
    canvas->translate(-bounds.centerX(), -bounds.centerY());
}


void render_path(SkCanvas* canvas, const SkPath& path) {
    canvas->clear(0xFFFFFFFF);

    const SkRect& bounds = path.getBounds();
    if (bounds.isEmpty()) {
        return;
    }

    SkAutoCanvasRestore acr(canvas, true);
    xlate_and_scale_to_bounds(canvas, bounds);

    SkPaint p;
    p.setColor(SK_ColorBLACK);
    p.setStyle(SkPaint::kStroke_Style);

    canvas->drawPath(path, p);
}

void render_bitmap(SkCanvas* canvas, const SkBitmap& input, const SkRect* srcRect = nullptr) {
    const SkISize& size = canvas->getDeviceSize();

    SkScalar xScale = SkIntToScalar(size.fWidth-2) / input.width();
    SkScalar yScale = SkIntToScalar(size.fHeight-2) / input.height();

    if (input.width() > input.height()) {
        yScale *= input.height() / (float) input.width();
    } else {
        xScale *= input.width() / (float) input.height();
    }

    SkRect dst = SkRect::MakeXYWH(SK_Scalar1, SK_Scalar1,
                                  xScale * input.width(),
                                  yScale * input.height());

    static const int kNumBlocks = 8;

    canvas->clear(0xFFFFFFFF);
    SkISize block = {
        canvas->imageInfo().width()/kNumBlocks,
        canvas->imageInfo().height()/kNumBlocks
    };
    for (int y = 0; y < kNumBlocks; ++y) {
        for (int x = 0; x < kNumBlocks; ++x) {
            SkPaint paint;
            paint.setColor((x+y)%2 ? SK_ColorLTGRAY : SK_ColorDKGRAY);
            SkRect r = SkRect::MakeXYWH(SkIntToScalar(x*block.width()),
                                        SkIntToScalar(y*block.height()),
                                        SkIntToScalar(block.width()),
                                        SkIntToScalar(block.height()));
            canvas->drawRect(r, paint);
        }
    }

    canvas->drawBitmapRect(input, dst, nullptr);

    if (srcRect) {
        SkRect r = SkRect::MakeLTRB(srcRect->fLeft * xScale + SK_Scalar1,
                                    srcRect->fTop * yScale + SK_Scalar1,
                                    srcRect->fRight * xScale + SK_Scalar1,
                                    srcRect->fBottom * yScale + SK_Scalar1);
        SkPaint p;
        p.setColor(SK_ColorRED);
        p.setStyle(SkPaint::kStroke_Style);

        canvas->drawRect(r, p);
    }
}

void render_rrect(SkCanvas* canvas, const SkRRect& rrect) {
    canvas->clear(0xFFFFFFFF);
    canvas->save();

    const SkRect& bounds = rrect.getBounds();

    xlate_and_scale_to_bounds(canvas, bounds);

    SkPaint p;
    p.setColor(SK_ColorBLACK);
    p.setStyle(SkPaint::kStroke_Style);

    canvas->drawRRect(rrect, p);
    canvas->restore();
}

void render_drrect(SkCanvas* canvas, const SkRRect& outer, const SkRRect& inner) {
    canvas->clear(0xFFFFFFFF);
    canvas->save();

    const SkRect& bounds = outer.getBounds();

    xlate_and_scale_to_bounds(canvas, bounds);

    SkPaint p;
    p.setColor(SK_ColorBLACK);
    p.setStyle(SkPaint::kStroke_Style);

    canvas->drawDRRect(outer, inner, p);
    canvas->restore();
}

};

static Json::Value make_json_point(const SkPoint& point) {
    Json::Value result(Json::arrayValue);
    result.append(Json::Value(point.x()));
    result.append(Json::Value(point.y()));
    return result;
}

static Json::Value make_json_point(SkScalar x, SkScalar y) {
    Json::Value result(Json::arrayValue);
    result.append(Json::Value(x));
    result.append(Json::Value(y));
    return result;
}

static Json::Value make_json_rect(const SkRect& rect) {
    Json::Value result(Json::arrayValue);
    result.append(Json::Value(rect.left()));
    result.append(Json::Value(rect.top()));
    result.append(Json::Value(rect.right()));
    result.append(Json::Value(rect.bottom()));
    return result;
}

static Json::Value make_json_irect(const SkIRect& rect) {
    Json::Value result(Json::arrayValue);
    result.append(Json::Value(rect.left()));
    result.append(Json::Value(rect.top()));
    result.append(Json::Value(rect.right()));
    result.append(Json::Value(rect.bottom()));
    return result;
}

static Json::Value make_json_rrect(const SkRRect& rrect) {
    Json::Value result(Json::arrayValue);
    result.append(make_json_rect(rrect.rect()));
    result.append(make_json_point(rrect.radii(SkRRect::kUpperLeft_Corner)));
    result.append(make_json_point(rrect.radii(SkRRect::kUpperRight_Corner)));
    result.append(make_json_point(rrect.radii(SkRRect::kLowerRight_Corner)));
    result.append(make_json_point(rrect.radii(SkRRect::kLowerLeft_Corner)));
    return result;
}

static Json::Value make_json_matrix(const SkMatrix& matrix) {
    Json::Value result(Json::arrayValue);
    Json::Value row1(Json::arrayValue);
    row1.append(Json::Value(matrix[0]));
    row1.append(Json::Value(matrix[1]));
    row1.append(Json::Value(matrix[2]));
    result.append(row1);
    Json::Value row2(Json::arrayValue);
    row2.append(Json::Value(matrix[3]));
    row2.append(Json::Value(matrix[4]));
    row2.append(Json::Value(matrix[5]));
    result.append(row2);
    Json::Value row3(Json::arrayValue);
    row3.append(Json::Value(matrix[6]));
    row3.append(Json::Value(matrix[7]));
    row3.append(Json::Value(matrix[8]));
    result.append(row3);
    return result;
}
static Json::Value make_json_path(const SkPath& path) {
    Json::Value result(Json::objectValue);
    switch (path.getFillType()) {
        case SkPath::kWinding_FillType:
            result[SKDEBUGCANVAS_ATTRIBUTE_FILLTYPE] = SKDEBUGCANVAS_FILLTYPE_WINDING;
            break;
        case SkPath::kEvenOdd_FillType:
            result[SKDEBUGCANVAS_ATTRIBUTE_FILLTYPE] = SKDEBUGCANVAS_FILLTYPE_EVENODD;
            break;
        case SkPath::kInverseWinding_FillType:
            result[SKDEBUGCANVAS_ATTRIBUTE_FILLTYPE] = SKDEBUGCANVAS_FILLTYPE_INVERSEWINDING;
            break;
        case SkPath::kInverseEvenOdd_FillType:
            result[SKDEBUGCANVAS_ATTRIBUTE_FILLTYPE] = SKDEBUGCANVAS_FILLTYPE_INVERSEEVENODD;
            break;
    }    
    Json::Value verbs(Json::arrayValue);
    SkPath::Iter iter(path, false);
    SkPoint pts[4];
    SkPath::Verb verb;
    while ((verb = iter.next(pts)) != SkPath::kDone_Verb) {
        switch (verb) {
            case SkPath::kLine_Verb: {
                Json::Value line(Json::objectValue);
                line[SKDEBUGCANVAS_VERB_LINE] = make_json_point(pts[1]);
                verbs.append(line);
                break;
            }
            case SkPath::kQuad_Verb: {
                Json::Value quad(Json::objectValue);
                Json::Value coords(Json::arrayValue);
                coords.append(make_json_point(pts[1]));
                coords.append(make_json_point(pts[2]));
                quad[SKDEBUGCANVAS_VERB_QUAD] = coords;
                verbs.append(quad);
                break;
            }
            case SkPath::kCubic_Verb: {
                Json::Value cubic(Json::objectValue);
                Json::Value coords(Json::arrayValue);
                coords.append(make_json_point(pts[1]));
                coords.append(make_json_point(pts[2]));
                coords.append(make_json_point(pts[3]));
                cubic[SKDEBUGCANVAS_VERB_CUBIC] = coords;
                verbs.append(cubic);
                break;
            }
            case SkPath::kConic_Verb: {
                Json::Value conic(Json::objectValue);
                Json::Value coords(Json::arrayValue);
                coords.append(make_json_point(pts[1]));
                coords.append(make_json_point(pts[2]));
                coords.append(Json::Value(iter.conicWeight()));
                conic[SKDEBUGCANVAS_VERB_CONIC] = coords;
                verbs.append(conic);
                break;
            }
            case SkPath::kMove_Verb: {
                Json::Value move(Json::objectValue);
                move[SKDEBUGCANVAS_VERB_MOVE] = make_json_point(pts[0]);
                verbs.append(move);
                break;
            }
            case SkPath::kClose_Verb:
                verbs.append(Json::Value(SKDEBUGCANVAS_VERB_CLOSE));
                break;
            case SkPath::kDone_Verb:
                break;
        }
    }
    result[SKDEBUGCANVAS_ATTRIBUTE_VERBS] = verbs;
    return result;
}

static Json::Value make_json_region(const SkRegion& region) {
    return Json::Value("<unimplemented>");
}

static Json::Value make_json_regionop(SkRegion::Op op) {
    switch (op) {
        case SkRegion::kDifference_Op:
            return Json::Value(SKDEBUGCANVAS_REGIONOP_DIFFERENCE);
        case SkRegion::kIntersect_Op:
            return Json::Value(SKDEBUGCANVAS_REGIONOP_INTERSECT);
        case SkRegion::kUnion_Op:
            return Json::Value(SKDEBUGCANVAS_REGIONOP_UNION);
        case SkRegion::kXOR_Op:
            return Json::Value(SKDEBUGCANVAS_REGIONOP_XOR);
        case SkRegion::kReverseDifference_Op:
            return Json::Value(SKDEBUGCANVAS_REGIONOP_REVERSE_DIFFERENCE);
        case SkRegion::kReplace_Op:
            return Json::Value(SKDEBUGCANVAS_REGIONOP_REPLACE);
        default:
            SkASSERT(false);
            return Json::Value("<invalid region op>");
    };
}

static Json::Value make_json_pointmode(SkCanvas::PointMode mode) {
    switch (mode) {
        case SkCanvas::kPoints_PointMode:
            return Json::Value(SKDEBUGCANVAS_POINTMODE_POINTS);
        case SkCanvas::kLines_PointMode:
            return Json::Value(SKDEBUGCANVAS_POINTMODE_LINES);
        case SkCanvas::kPolygon_PointMode: 
            return Json::Value(SKDEBUGCANVAS_POINTMODE_POLYGON);
        default:
            SkASSERT(false);
            return Json::Value("<invalid point mode>");
    };
}

void store_scalar(Json::Value* target, const char* key, SkScalar value, SkScalar defaultValue) {
    if (value != defaultValue) {
        (*target)[key] = Json::Value(value);
    }
}

void store_bool(Json::Value* target, const char* key, bool value, bool defaultValue) {
    if (value != defaultValue) {
        (*target)[key] = Json::Value(value);
    }
}

static void encode_data(const void* data, size_t count, Json::Value* target) {
    // just use a brain-dead JSON array for now, switch to base64 or something else smarter down the
    // road
    for (size_t i = 0; i < count; i++) {
        target->append(((const uint8_t*)data)[i]);
    }
}

static void flatten(const SkFlattenable* flattenable, Json::Value* target, bool sendBinaries) {
    if (sendBinaries) {
        SkWriteBuffer buffer;
        flattenable->flatten(buffer);
        void* data = sk_malloc_throw(buffer.bytesWritten());
        buffer.writeToMemory(data);
        Json::Value bytes;
        encode_data(data, buffer.bytesWritten(), &bytes);
        Json::Value jsonFlattenable;
        jsonFlattenable[SKDEBUGCANVAS_ATTRIBUTE_NAME] = Json::Value(flattenable->getTypeName());
        jsonFlattenable[SKDEBUGCANVAS_ATTRIBUTE_BYTES] = bytes;
        (*target) = jsonFlattenable;
        free(data);
    } else {
        (*target)[SKDEBUGCANVAS_ATTRIBUTE_DESCRIPTION] = Json::Value(flattenable->getTypeName());
    }
}

static bool SK_WARN_UNUSED_RESULT flatten(const SkImage& image, Json::Value* target, 
                                          bool sendBinaries) {
    if (sendBinaries) {
        SkData* encoded = image.encode(SkImageEncoder::kPNG_Type, 100);
        if (encoded == nullptr) {
            // PNG encode doesn't necessarily support all color formats, convert to a different
            // format
            size_t rowBytes = 4 * image.width();
            void* buffer = sk_malloc_throw(rowBytes * image.height());
            SkImageInfo dstInfo = SkImageInfo::Make(image.width(), image.height(), 
                                                    kN32_SkColorType, kPremul_SkAlphaType);
            if (!image.readPixels(dstInfo, buffer, rowBytes, 0, 0)) {
                SkDebugf("readPixels failed\n");
                return false;
            }
            SkImage* converted = SkImage::NewRasterCopy(dstInfo, buffer, rowBytes);
            encoded = converted->encode(SkImageEncoder::kPNG_Type, 100);
            if (encoded == nullptr) {
                SkDebugf("image encode failed\n");
                return false;
            }
            free(converted);
            free(buffer);
        }
        Json::Value bytes;
        encode_data(encoded->data(), encoded->size(), &bytes);
        (*target)[SKDEBUGCANVAS_ATTRIBUTE_BYTES] = bytes;
        encoded->unref();
    } else {
        SkString description = SkStringPrintf("%dx%d pixel image", image.width(), image.height());
        (*target)[SKDEBUGCANVAS_ATTRIBUTE_DESCRIPTION] = Json::Value(description.c_str());
    }
    return true;
}

static const char* color_type_name(SkColorType colorType) {
    switch (colorType) {
        case kARGB_4444_SkColorType:
            return SKDEBUGCANVAS_COLORTYPE_ARGB4444;
        case kRGBA_8888_SkColorType:
            return SKDEBUGCANVAS_COLORTYPE_RGBA8888;
        case kBGRA_8888_SkColorType:
            return SKDEBUGCANVAS_COLORTYPE_BGRA8888;
        case kRGB_565_SkColorType:
            return SKDEBUGCANVAS_COLORTYPE_565;
        case kGray_8_SkColorType:
            return SKDEBUGCANVAS_COLORTYPE_GRAY8;
        case kIndex_8_SkColorType:
            return SKDEBUGCANVAS_COLORTYPE_INDEX8;
        case kAlpha_8_SkColorType:
            return SKDEBUGCANVAS_COLORTYPE_ALPHA8;
        default:
            SkASSERT(false);
            return SKDEBUGCANVAS_COLORTYPE_RGBA8888;
    }
}

static const char* alpha_type_name(SkAlphaType alphaType) {
    switch (alphaType) {
        case kOpaque_SkAlphaType:
            return SKDEBUGCANVAS_ALPHATYPE_OPAQUE;
        case kPremul_SkAlphaType:
            return SKDEBUGCANVAS_ALPHATYPE_PREMUL;
        case kUnpremul_SkAlphaType:
            return SKDEBUGCANVAS_ALPHATYPE_UNPREMUL;
        default:
            SkASSERT(false);
            return SKDEBUGCANVAS_ALPHATYPE_OPAQUE;
    }
}

// note that the caller is responsible for freeing the pointer
static Json::ArrayIndex decode_data(Json::Value bytes, void** target) {
    Json::ArrayIndex size = bytes.size();
    *target = sk_malloc_throw(size);
    for (Json::ArrayIndex i = 0; i < size; i++) {
        ((uint8_t*) *target)[i] = bytes[i].asInt();
    }
    return size;
}

static SkFlattenable* load_flattenable(Json::Value jsonFlattenable) {
    if (!jsonFlattenable.isMember(SKDEBUGCANVAS_ATTRIBUTE_NAME)) {
        return nullptr;
    }
    const char* name = jsonFlattenable[SKDEBUGCANVAS_ATTRIBUTE_NAME].asCString();
    SkFlattenable::Factory factory = SkFlattenable::NameToFactory(name);
    if (factory == nullptr) {
        SkDebugf("no factory for loading '%s'\n", name);
        return nullptr;
    }
    void* data;
    int size = decode_data(jsonFlattenable[SKDEBUGCANVAS_ATTRIBUTE_BYTES], &data);
    SkValidatingReadBuffer buffer(data, size);
    SkFlattenable* result = factory(buffer);
    free(data);
    if (!buffer.isValid()) {
        SkDebugf("invalid buffer loading flattenable\n");
        return nullptr;
    }
    return result;
}

static SkColorType colortype_from_name(const char* name) {
    if (!strcmp(name, SKDEBUGCANVAS_COLORTYPE_ARGB4444)) {
        return kARGB_4444_SkColorType;
    }
    else if (!strcmp(name, SKDEBUGCANVAS_COLORTYPE_RGBA8888)) {
        return kRGBA_8888_SkColorType;
    }
    else if (!strcmp(name, SKDEBUGCANVAS_COLORTYPE_BGRA8888)) {
        return kBGRA_8888_SkColorType;
    }
    else if (!strcmp(name, SKDEBUGCANVAS_COLORTYPE_565)) {
        return kRGB_565_SkColorType;
    }
    else if (!strcmp(name, SKDEBUGCANVAS_COLORTYPE_GRAY8)) {
        return kGray_8_SkColorType;
    }
    else if (!strcmp(name, SKDEBUGCANVAS_COLORTYPE_INDEX8)) {
        return kIndex_8_SkColorType;
    }
    else if (!strcmp(name, SKDEBUGCANVAS_COLORTYPE_ALPHA8)) {
        return kAlpha_8_SkColorType;
    }
    SkASSERT(false);
    return kN32_SkColorType;
}

static SkBitmap* convert_colortype(SkBitmap* bitmap, SkColorType colorType) {
    if (bitmap->colorType() == colorType  ) {
        return bitmap;
    }
    SkBitmap* dst = new SkBitmap();
    if (bitmap->copyTo(dst, colorType)) {
        delete bitmap;
        return dst;
    }
    SkASSERT(false);
    delete dst;
    return bitmap;
}

// caller is responsible for freeing return value
static SkBitmap* load_bitmap(const Json::Value& jsonBitmap) {
    if (!jsonBitmap.isMember(SKDEBUGCANVAS_ATTRIBUTE_BYTES)) {
        SkDebugf("invalid bitmap\n");
        return nullptr;
    }
    void* data;
    int size = decode_data(jsonBitmap[SKDEBUGCANVAS_ATTRIBUTE_BYTES], &data);
    SkMemoryStream stream(data, size);
    SkImageDecoder* decoder = SkImageDecoder::Factory(&stream);
    SkBitmap* bitmap = new SkBitmap();
    SkImageDecoder::Result result = decoder->decode(&stream, bitmap, 
                                                    SkImageDecoder::kDecodePixels_Mode);
    free(decoder);
    if (result != SkImageDecoder::kFailure) {
        free(data);
        if (jsonBitmap.isMember(SKDEBUGCANVAS_ATTRIBUTE_COLOR)) {
            const char* ctName = jsonBitmap[SKDEBUGCANVAS_ATTRIBUTE_COLOR].asCString();
            SkColorType ct = colortype_from_name(ctName);
            if (ct != kIndex_8_SkColorType) {
                bitmap = convert_colortype(bitmap, ct);
            }
        }
        return bitmap;
    }
    SkDebugf("image decode failed\n");
    free(data);
    return nullptr;
}

static SkImage* load_image(const Json::Value& jsonImage) {
    SkBitmap* bitmap = load_bitmap(jsonImage);
    if (bitmap == nullptr) {
        return nullptr;
    }
    SkImage* result = SkImage::NewFromBitmap(*bitmap);
    delete bitmap;
    return result;
}

static bool SK_WARN_UNUSED_RESULT flatten(const SkBitmap& bitmap, Json::Value* target, 
                                          bool sendBinaries) {
    bitmap.lockPixels();
    SkAutoTUnref<SkImage> image(SkImage::NewFromBitmap(bitmap));
    bitmap.unlockPixels();
    (*target)[SKDEBUGCANVAS_ATTRIBUTE_COLOR] = Json::Value(color_type_name(bitmap.colorType()));
    (*target)[SKDEBUGCANVAS_ATTRIBUTE_ALPHA] = Json::Value(alpha_type_name(bitmap.alphaType()));
    bool success = flatten(*image, target, sendBinaries);
    return success;
}

static void apply_paint_color(const SkPaint& paint, Json::Value* target) {
    SkColor color = paint.getColor();
    if (color != SK_ColorBLACK) {
        Json::Value colorValue(Json::arrayValue);
        colorValue.append(Json::Value(SkColorGetA(color)));
        colorValue.append(Json::Value(SkColorGetR(color)));
        colorValue.append(Json::Value(SkColorGetG(color)));
        colorValue.append(Json::Value(SkColorGetB(color)));
        (*target)[SKDEBUGCANVAS_ATTRIBUTE_COLOR] = colorValue;;
    }
}

static void apply_paint_style(const SkPaint& paint, Json::Value* target) {
    SkPaint::Style style = paint.getStyle();
    if (style != SkPaint::kFill_Style) {
        switch (style) {
            case SkPaint::kStroke_Style: {
                Json::Value stroke(SKDEBUGCANVAS_STYLE_STROKE);
                (*target)[SKDEBUGCANVAS_ATTRIBUTE_STYLE] = stroke;
                break;
            }
            case SkPaint::kStrokeAndFill_Style: {
                Json::Value strokeAndFill(SKDEBUGCANVAS_STYLE_STROKEANDFILL);
                (*target)[SKDEBUGCANVAS_ATTRIBUTE_STYLE] = strokeAndFill;
                break;
            }
            default: SkASSERT(false);
        }
    }
}

static void apply_paint_cap(const SkPaint& paint, Json::Value* target) {
    SkPaint::Cap cap = paint.getStrokeCap();
    if (cap != SkPaint::kDefault_Cap) {
        switch (cap) {
            case SkPaint::kButt_Cap: {
                (*target)[SKDEBUGCANVAS_ATTRIBUTE_CAP] = Json::Value(SKDEBUGCANVAS_CAP_BUTT);
                break;
            }
            case SkPaint::kRound_Cap: {
                (*target)[SKDEBUGCANVAS_ATTRIBUTE_CAP] = Json::Value(SKDEBUGCANVAS_CAP_ROUND);
                break;
            }
            case SkPaint::kSquare_Cap: {
                (*target)[SKDEBUGCANVAS_ATTRIBUTE_CAP] = Json::Value(SKDEBUGCANVAS_CAP_SQUARE);
                break;
            }
            default: SkASSERT(false);
        }
    }
}
static void apply_paint_maskfilter(const SkPaint& paint, Json::Value* target, bool sendBinaries) {
    SkMaskFilter* maskFilter = paint.getMaskFilter();
    if (maskFilter != nullptr) {
        SkMaskFilter::BlurRec blurRec;
        if (maskFilter->asABlur(&blurRec)) {
            Json::Value blur(Json::objectValue);
            blur[SKDEBUGCANVAS_ATTRIBUTE_SIGMA] = Json::Value(blurRec.fSigma);
            switch (blurRec.fStyle) {
                case SkBlurStyle::kNormal_SkBlurStyle:
                    blur[SKDEBUGCANVAS_ATTRIBUTE_STYLE] = Json::Value(SKDEBUGCANVAS_BLURSTYLE_NORMAL);
                    break;
                case SkBlurStyle::kSolid_SkBlurStyle:
                    blur[SKDEBUGCANVAS_ATTRIBUTE_STYLE] = Json::Value(SKDEBUGCANVAS_BLURSTYLE_SOLID);
                    break;
                case SkBlurStyle::kOuter_SkBlurStyle:
                    blur[SKDEBUGCANVAS_ATTRIBUTE_STYLE] = Json::Value(SKDEBUGCANVAS_BLURSTYLE_OUTER);
                    break;
                case SkBlurStyle::kInner_SkBlurStyle:
                    blur[SKDEBUGCANVAS_ATTRIBUTE_STYLE] = Json::Value(SKDEBUGCANVAS_BLURSTYLE_INNER);
                    break;
                default:
                    SkASSERT(false);
            }
            switch (blurRec.fQuality) {
                case SkBlurQuality::kLow_SkBlurQuality:
                    blur[SKDEBUGCANVAS_ATTRIBUTE_QUALITY] = Json::Value(SKDEBUGCANVAS_BLURQUALITY_LOW);
                    break;
                case SkBlurQuality::kHigh_SkBlurQuality:
                    blur[SKDEBUGCANVAS_ATTRIBUTE_QUALITY] = Json::Value(SKDEBUGCANVAS_BLURQUALITY_HIGH);
                    break;
                default:
                    SkASSERT(false);
            }
            (*target)[SKDEBUGCANVAS_ATTRIBUTE_BLUR] = blur;
        } else {
            Json::Value jsonMaskFilter;
            flatten(maskFilter, &jsonMaskFilter, sendBinaries);
            (*target)[SKDEBUGCANVAS_ATTRIBUTE_MASKFILTER] = jsonMaskFilter;
        }
    }
}

static void apply_paint_patheffect(const SkPaint& paint, Json::Value* target, bool sendBinaries) {
    SkPathEffect* pathEffect = paint.getPathEffect();
    if (pathEffect != nullptr) {
        SkPathEffect::DashInfo dashInfo;
        SkPathEffect::DashType dashType = pathEffect->asADash(&dashInfo);
        if (dashType == SkPathEffect::kDash_DashType) {
            dashInfo.fIntervals = (SkScalar*) sk_malloc_throw(dashInfo.fCount * sizeof(SkScalar));
            pathEffect->asADash(&dashInfo);
            Json::Value dashing(Json::objectValue);
            Json::Value intervals(Json::arrayValue);
            for (int32_t i = 0; i < dashInfo.fCount; i++) {
                intervals.append(Json::Value(dashInfo.fIntervals[i]));
            }
            free(dashInfo.fIntervals);
            dashing[SKDEBUGCANVAS_ATTRIBUTE_INTERVALS] = intervals;
            dashing[SKDEBUGCANVAS_ATTRIBUTE_PHASE] = dashInfo.fPhase;
            (*target)[SKDEBUGCANVAS_ATTRIBUTE_DASHING] = dashing;
        } else {
            Json::Value jsonPathEffect;
            flatten(pathEffect, &jsonPathEffect, sendBinaries);
            (*target)[SKDEBUGCANVAS_ATTRIBUTE_PATHEFFECT] = jsonPathEffect;
        }
    }
}
    
static void apply_paint_textalign(const SkPaint& paint, Json::Value* target) {
    SkPaint::Align textAlign = paint.getTextAlign();
    if (textAlign != SkPaint::kLeft_Align) {
        switch (textAlign) {
            case SkPaint::kCenter_Align: {
                (*target)[SKDEBUGCANVAS_ATTRIBUTE_TEXTALIGN] = SKDEBUGCANVAS_ALIGN_CENTER;
                break;
            }
            case SkPaint::kRight_Align: {
                (*target)[SKDEBUGCANVAS_ATTRIBUTE_TEXTALIGN] = SKDEBUGCANVAS_ALIGN_RIGHT;
                break;
            }
            default: SkASSERT(false);
        }
    }
}

static void apply_paint_typeface(const SkPaint& paint, Json::Value* target, 
                                 bool sendBinaries) {
    SkTypeface* typeface = paint.getTypeface();
    if (typeface != nullptr) {
        if (sendBinaries) {
            Json::Value jsonTypeface;
            SkDynamicMemoryWStream buffer;
            typeface->serialize(&buffer);
            void* data = sk_malloc_throw(buffer.bytesWritten());
            buffer.copyTo(data);
            Json::Value bytes;
            encode_data(data, buffer.bytesWritten(), &bytes);
            jsonTypeface[SKDEBUGCANVAS_ATTRIBUTE_BYTES] = bytes;
            free(data);
            (*target)[SKDEBUGCANVAS_ATTRIBUTE_TYPEFACE] = jsonTypeface;
        }
    }
}

static void apply_paint_shader(const SkPaint& paint, Json::Value* target, bool sendBinaries) {
    SkFlattenable* shader = paint.getShader();
    if (shader != nullptr) {
        Json::Value jsonShader;
        flatten(shader, &jsonShader, sendBinaries);
        (*target)[SKDEBUGCANVAS_ATTRIBUTE_SHADER] = jsonShader;
    }
}

static void apply_paint_xfermode(const SkPaint& paint, Json::Value* target, bool sendBinaries) {
    SkFlattenable* xfermode = paint.getXfermode();
    if (xfermode != nullptr) {
        Json::Value jsonXfermode;
        flatten(xfermode, &jsonXfermode, sendBinaries);
        (*target)[SKDEBUGCANVAS_ATTRIBUTE_XFERMODE] = jsonXfermode;
    }
}

static void apply_paint_imagefilter(const SkPaint& paint, Json::Value* target, bool sendBinaries) {
    SkFlattenable* imageFilter = paint.getImageFilter();
    if (imageFilter != nullptr) {
        Json::Value jsonImageFilter;
        flatten(imageFilter, &jsonImageFilter, sendBinaries);
        (*target)[SKDEBUGCANVAS_ATTRIBUTE_IMAGEFILTER] = jsonImageFilter;
    }
}

static void apply_paint_colorfilter(const SkPaint& paint, Json::Value* target, bool sendBinaries) {
    SkFlattenable* colorFilter = paint.getColorFilter();
    if (colorFilter != nullptr) {
        Json::Value jsonColorFilter;
        flatten(colorFilter, &jsonColorFilter, sendBinaries);
        (*target)[SKDEBUGCANVAS_ATTRIBUTE_COLORFILTER] = jsonColorFilter;
    }
}

Json::Value make_json_paint(const SkPaint& paint, bool sendBinaries) {
    Json::Value result(Json::objectValue);
    store_scalar(&result, SKDEBUGCANVAS_ATTRIBUTE_STROKEWIDTH, paint.getStrokeWidth(), 0.0f);
    store_scalar(&result, SKDEBUGCANVAS_ATTRIBUTE_STROKEMITER, paint.getStrokeMiter(), 
                 SkPaintDefaults_MiterLimit);
    store_bool(&result, SKDEBUGCANVAS_ATTRIBUTE_ANTIALIAS, paint.isAntiAlias(), false);
    store_scalar(&result, SKDEBUGCANVAS_ATTRIBUTE_TEXTSIZE, paint.getTextSize(), 
                 SkPaintDefaults_TextSize);
    store_scalar(&result, SKDEBUGCANVAS_ATTRIBUTE_TEXTSCALEX, paint.getTextScaleX(), SK_Scalar1);
    store_scalar(&result, SKDEBUGCANVAS_ATTRIBUTE_TEXTSCALEX, paint.getTextSkewX(), 0.0f);
    apply_paint_color(paint, &result);
    apply_paint_style(paint, &result);
    apply_paint_cap(paint, &result);
    apply_paint_textalign(paint, &result);
    apply_paint_patheffect(paint, &result, sendBinaries);
    apply_paint_maskfilter(paint, &result, sendBinaries);
    apply_paint_shader(paint, &result, sendBinaries);
    apply_paint_xfermode(paint, &result, sendBinaries);
    apply_paint_imagefilter(paint, &result, sendBinaries);
    apply_paint_colorfilter(paint, &result, sendBinaries);
    apply_paint_typeface(paint, &result, sendBinaries);
    return result;
}

static void extract_json_paint_color(Json::Value& jsonPaint, SkPaint* target) {
    if (jsonPaint.isMember(SKDEBUGCANVAS_ATTRIBUTE_COLOR)) {
        Json::Value color = jsonPaint[SKDEBUGCANVAS_ATTRIBUTE_COLOR];
        target->setColor(SkColorSetARGB(color[0].asInt(), color[1].asInt(), color[2].asInt(),
                         color[3].asInt()));
    }
}

static void extract_json_paint_shader(Json::Value& jsonPaint, SkPaint* target) {
    if (jsonPaint.isMember(SKDEBUGCANVAS_ATTRIBUTE_SHADER)) {
        Json::Value jsonShader = jsonPaint[SKDEBUGCANVAS_ATTRIBUTE_SHADER];
        SkShader* shader = (SkShader*) load_flattenable(jsonShader);
        if (shader != nullptr) {
            target->setShader(shader);
            shader->unref();
        }
    }
}

static void extract_json_paint_patheffect(Json::Value& jsonPaint, SkPaint* target) {
    if (jsonPaint.isMember(SKDEBUGCANVAS_ATTRIBUTE_PATHEFFECT)) {
        Json::Value jsonPathEffect = jsonPaint[SKDEBUGCANVAS_ATTRIBUTE_PATHEFFECT];
        SkPathEffect* pathEffect = (SkPathEffect*) load_flattenable(jsonPathEffect);
        if (pathEffect != nullptr) {
            target->setPathEffect(pathEffect);
            pathEffect->unref();
        }
    }
}

static void extract_json_paint_maskfilter(Json::Value& jsonPaint, SkPaint* target) {
    if (jsonPaint.isMember(SKDEBUGCANVAS_ATTRIBUTE_MASKFILTER)) {
        Json::Value jsonMaskFilter = jsonPaint[SKDEBUGCANVAS_ATTRIBUTE_MASKFILTER];
        SkMaskFilter* maskFilter = (SkMaskFilter*) load_flattenable(jsonMaskFilter);
        if (maskFilter != nullptr) {
            target->setMaskFilter(maskFilter);
            maskFilter->unref();
        }
    }
}

static void extract_json_paint_colorfilter(Json::Value& jsonPaint, SkPaint* target) {
    if (jsonPaint.isMember(SKDEBUGCANVAS_ATTRIBUTE_COLORFILTER)) {
        Json::Value jsonColorFilter = jsonPaint[SKDEBUGCANVAS_ATTRIBUTE_COLORFILTER];
        SkColorFilter* colorFilter = (SkColorFilter*) load_flattenable(jsonColorFilter);
        if (colorFilter != nullptr) {
            target->setColorFilter(colorFilter);
            colorFilter->unref();
        }
    }
}

static void extract_json_paint_xfermode(Json::Value& jsonPaint, SkPaint* target) {
    if (jsonPaint.isMember(SKDEBUGCANVAS_ATTRIBUTE_XFERMODE)) {
        Json::Value jsonXfermode = jsonPaint[SKDEBUGCANVAS_ATTRIBUTE_XFERMODE];
        SkXfermode* xfermode = (SkXfermode*) load_flattenable(jsonXfermode);
        if (xfermode != nullptr) {
            target->setXfermode(xfermode);
            xfermode->unref();
        }
    }
}

static void extract_json_paint_imagefilter(Json::Value& jsonPaint, SkPaint* target) {
    if (jsonPaint.isMember(SKDEBUGCANVAS_ATTRIBUTE_IMAGEFILTER)) {
        Json::Value jsonImageFilter = jsonPaint[SKDEBUGCANVAS_ATTRIBUTE_IMAGEFILTER];
        SkImageFilter* imageFilter = (SkImageFilter*) load_flattenable(jsonImageFilter);
        if (imageFilter != nullptr) {
            target->setImageFilter(imageFilter);
            imageFilter->unref();
        }
    }
}

static void extract_json_paint_style(Json::Value& jsonPaint, SkPaint* target) {
    if (jsonPaint.isMember(SKDEBUGCANVAS_ATTRIBUTE_STYLE)) {
        const char* style = jsonPaint[SKDEBUGCANVAS_ATTRIBUTE_STYLE].asCString();
        if (!strcmp(style, SKDEBUGCANVAS_STYLE_FILL)) {
            target->setStyle(SkPaint::kFill_Style);
        }
        else if (!strcmp(style, SKDEBUGCANVAS_STYLE_STROKE)) {
            target->setStyle(SkPaint::kStroke_Style);
        }
        else if (!strcmp(style, SKDEBUGCANVAS_STYLE_STROKEANDFILL)) {
            target->setStyle(SkPaint::kStrokeAndFill_Style);
        }
    }
}

static void extract_json_paint_strokewidth(Json::Value& jsonPaint, SkPaint* target) {
    if (jsonPaint.isMember(SKDEBUGCANVAS_ATTRIBUTE_STROKEWIDTH)) {
        float strokeWidth = jsonPaint[SKDEBUGCANVAS_ATTRIBUTE_STROKEWIDTH].asFloat();
        target->setStrokeWidth(strokeWidth);
    }    
}

static void extract_json_paint_strokemiter(Json::Value& jsonPaint, SkPaint* target) {
    if (jsonPaint.isMember(SKDEBUGCANVAS_ATTRIBUTE_STROKEMITER)) {
        float strokeMiter = jsonPaint[SKDEBUGCANVAS_ATTRIBUTE_STROKEMITER].asFloat();
        target->setStrokeMiter(strokeMiter);
    }    
}

static void extract_json_paint_cap(Json::Value& jsonPaint, SkPaint* target) {
    if (jsonPaint.isMember(SKDEBUGCANVAS_ATTRIBUTE_CAP)) {
        const char* cap = jsonPaint[SKDEBUGCANVAS_ATTRIBUTE_CAP].asCString();
        if (!strcmp(cap, SKDEBUGCANVAS_CAP_BUTT)) {
            target->setStrokeCap(SkPaint::kButt_Cap);
        }
        else if (!strcmp(cap, SKDEBUGCANVAS_CAP_ROUND)) {
            target->setStrokeCap(SkPaint::kRound_Cap);
        }
        else if (!strcmp(cap, SKDEBUGCANVAS_CAP_SQUARE)) {
            target->setStrokeCap(SkPaint::kSquare_Cap);
        }
    }
}

static void extract_json_paint_antialias(Json::Value& jsonPaint, SkPaint* target) {
    if (jsonPaint.isMember(SKDEBUGCANVAS_ATTRIBUTE_ANTIALIAS)) {
        target->setAntiAlias(jsonPaint[SKDEBUGCANVAS_ATTRIBUTE_ANTIALIAS].asBool());
    }
}

static void extract_json_paint_blur(Json::Value& jsonPaint, SkPaint* target) {
    if (jsonPaint.isMember(SKDEBUGCANVAS_ATTRIBUTE_BLUR)) {
        Json::Value blur = jsonPaint[SKDEBUGCANVAS_ATTRIBUTE_BLUR];
        SkScalar sigma = blur[SKDEBUGCANVAS_ATTRIBUTE_SIGMA].asFloat();
        SkBlurStyle style;
        const char* jsonStyle = blur[SKDEBUGCANVAS_ATTRIBUTE_STYLE].asCString();
        if (!strcmp(jsonStyle, SKDEBUGCANVAS_BLURSTYLE_NORMAL)) {
            style = SkBlurStyle::kNormal_SkBlurStyle;
        }
        else if (!strcmp(jsonStyle, SKDEBUGCANVAS_BLURSTYLE_SOLID)) {
            style = SkBlurStyle::kSolid_SkBlurStyle;
        }
        else if (!strcmp(jsonStyle, SKDEBUGCANVAS_BLURSTYLE_OUTER)) {
            style = SkBlurStyle::kOuter_SkBlurStyle;
        }
        else if (!strcmp(jsonStyle, SKDEBUGCANVAS_BLURSTYLE_INNER)) {
            style = SkBlurStyle::kInner_SkBlurStyle;
        }
        else {
            SkASSERT(false);
            style = SkBlurStyle::kNormal_SkBlurStyle;
        }
        SkBlurMaskFilter::BlurFlags flags;
        const char* jsonQuality = blur[SKDEBUGCANVAS_ATTRIBUTE_QUALITY].asCString();
        if (!strcmp(jsonQuality, SKDEBUGCANVAS_BLURQUALITY_LOW)) {
            flags = SkBlurMaskFilter::BlurFlags::kNone_BlurFlag;
        }
        else if (!strcmp(jsonQuality, SKDEBUGCANVAS_BLURQUALITY_HIGH)) {
            flags = SkBlurMaskFilter::BlurFlags::kHighQuality_BlurFlag;
        }
        else {
            SkASSERT(false);
            flags = SkBlurMaskFilter::BlurFlags::kNone_BlurFlag;
        }
        target->setMaskFilter(SkBlurMaskFilter::Create(style, sigma, flags));
    }
}

static void extract_json_paint_dashing(Json::Value& jsonPaint, SkPaint* target) {
    if (jsonPaint.isMember(SKDEBUGCANVAS_ATTRIBUTE_DASHING)) {
        Json::Value dash = jsonPaint[SKDEBUGCANVAS_ATTRIBUTE_DASHING];
        Json::Value jsonIntervals = dash[SKDEBUGCANVAS_ATTRIBUTE_INTERVALS];
        Json::ArrayIndex count = jsonIntervals.size();
        SkScalar* intervals = (SkScalar*) sk_malloc_throw(count * sizeof(SkScalar));
        for (Json::ArrayIndex i = 0; i < count; i++) {
            intervals[i] = jsonIntervals[i].asFloat();
        }
        SkScalar phase = dash[SKDEBUGCANVAS_ATTRIBUTE_PHASE].asFloat();
        target->setPathEffect(SkDashPathEffect::Create(intervals, count, phase));
        free(intervals);
    }
}

static void extract_json_paint_textalign(Json::Value& jsonPaint, SkPaint* target) {
    if (jsonPaint.isMember(SKDEBUGCANVAS_ATTRIBUTE_TEXTALIGN)) {
        SkPaint::Align textAlign;
        const char* jsonAlign = jsonPaint[SKDEBUGCANVAS_ATTRIBUTE_TEXTALIGN].asCString();
        if (!strcmp(jsonAlign, SKDEBUGCANVAS_ALIGN_LEFT)) {
            textAlign = SkPaint::kLeft_Align;
        }
        else if (!strcmp(jsonAlign, SKDEBUGCANVAS_ALIGN_CENTER)) {
            textAlign = SkPaint::kCenter_Align;
        }
        else if (!strcmp(jsonAlign, SKDEBUGCANVAS_ALIGN_RIGHT)) {
            textAlign = SkPaint::kRight_Align;
        }
        else {
            SkASSERT(false);
            textAlign = SkPaint::kLeft_Align;
        }
        target->setTextAlign(textAlign);
    }
}

static void extract_json_paint_textsize(Json::Value& jsonPaint, SkPaint* target) {
    if (jsonPaint.isMember(SKDEBUGCANVAS_ATTRIBUTE_TEXTSIZE)) {
        float textSize = jsonPaint[SKDEBUGCANVAS_ATTRIBUTE_TEXTSIZE].asFloat();
        target->setTextSize(textSize);
    }
}

static void extract_json_paint_textscalex(Json::Value& jsonPaint, SkPaint* target) {
    if (jsonPaint.isMember(SKDEBUGCANVAS_ATTRIBUTE_TEXTSCALEX)) {
        float textScaleX = jsonPaint[SKDEBUGCANVAS_ATTRIBUTE_TEXTSCALEX].asFloat();
        target->setTextScaleX(textScaleX);
    }
}

static void extract_json_paint_textskewx(Json::Value& jsonPaint, SkPaint* target) {
    if (jsonPaint.isMember(SKDEBUGCANVAS_ATTRIBUTE_TEXTSKEWX)) {
        float textSkewX = jsonPaint[SKDEBUGCANVAS_ATTRIBUTE_TEXTSKEWX].asFloat();
        target->setTextSkewX(textSkewX);
    }
}

static void extract_json_paint_typeface(Json::Value& jsonPaint, SkPaint* target) {
    if (jsonPaint.isMember(SKDEBUGCANVAS_ATTRIBUTE_TYPEFACE)) {
        Json::Value jsonTypeface = jsonPaint[SKDEBUGCANVAS_ATTRIBUTE_TYPEFACE];
        Json::Value bytes = jsonTypeface[SKDEBUGCANVAS_ATTRIBUTE_BYTES];
        void* data;
        Json::ArrayIndex length = decode_data(bytes, &data);
        SkMemoryStream buffer(data, length);
        SkTypeface* typeface = SkTypeface::Deserialize(&buffer);
        free(data);
        target->setTypeface(typeface);
    }
}

static void extract_json_paint(Json::Value& paint, SkPaint* result) {
    extract_json_paint_color(paint, result);
    extract_json_paint_shader(paint, result);
    extract_json_paint_patheffect(paint, result);
    extract_json_paint_maskfilter(paint, result);
    extract_json_paint_colorfilter(paint, result);
    extract_json_paint_xfermode(paint, result);
    extract_json_paint_imagefilter(paint, result);
    extract_json_paint_style(paint, result);
    extract_json_paint_strokewidth(paint, result);
    extract_json_paint_strokemiter(paint, result);
    extract_json_paint_cap(paint, result);
    extract_json_paint_antialias(paint, result);
    extract_json_paint_blur(paint, result);
    extract_json_paint_dashing(paint, result);
    extract_json_paint_textalign(paint, result);
    extract_json_paint_textsize(paint, result);
    extract_json_paint_textscalex(paint, result);
    extract_json_paint_textskewx(paint, result);
    extract_json_paint_typeface(paint, result);
}

static void extract_json_rect(Json::Value& rect, SkRect* result) {
    result->set(rect[0].asFloat(), rect[1].asFloat(), rect[2].asFloat(), rect[3].asFloat());
}

static void extract_json_irect(Json::Value& rect, SkIRect* result) {
    result->set(rect[0].asInt(), rect[1].asInt(), rect[2].asInt(), rect[3].asInt());
}

static void extract_json_rrect(Json::Value& rrect, SkRRect* result) {
    SkVector radii[4] = {
                            { rrect[1][0].asFloat(), rrect[1][1].asFloat() }, 
                            { rrect[2][0].asFloat(), rrect[2][1].asFloat() }, 
                            { rrect[3][0].asFloat(), rrect[3][1].asFloat() }, 
                            { rrect[4][0].asFloat(), rrect[4][1].asFloat() }
                        };
    result->setRectRadii(SkRect::MakeLTRB(rrect[0][0].asFloat(), rrect[0][1].asFloat(), 
                                          rrect[0][2].asFloat(), rrect[0][3].asFloat()), 
                                          radii);
}

static void extract_json_matrix(Json::Value& matrix, SkMatrix* result) {
    SkScalar values[] = { 
        matrix[0][0].asFloat(), matrix[0][1].asFloat(), matrix[0][2].asFloat(),
        matrix[1][0].asFloat(), matrix[1][1].asFloat(), matrix[1][2].asFloat(),
        matrix[2][0].asFloat(), matrix[2][1].asFloat(), matrix[2][2].asFloat() 
    };
    result->set9(values);
}

static void extract_json_path(Json::Value& path, SkPath* result) {
    const char* fillType = path[SKDEBUGCANVAS_ATTRIBUTE_FILLTYPE].asCString();
    if (!strcmp(fillType, SKDEBUGCANVAS_FILLTYPE_WINDING)) {
        result->setFillType(SkPath::kWinding_FillType);
    }
    else if (!strcmp(fillType, SKDEBUGCANVAS_FILLTYPE_EVENODD)) {
        result->setFillType(SkPath::kEvenOdd_FillType);
    }
    else if (!strcmp(fillType, SKDEBUGCANVAS_FILLTYPE_INVERSEWINDING)) {
        result->setFillType(SkPath::kInverseWinding_FillType);
    }
    else if (!strcmp(fillType, SKDEBUGCANVAS_FILLTYPE_INVERSEEVENODD)) {
        result->setFillType(SkPath::kInverseEvenOdd_FillType);
    }
    Json::Value verbs = path[SKDEBUGCANVAS_ATTRIBUTE_VERBS];
    for (Json::ArrayIndex i = 0; i < verbs.size(); i++) {
        Json::Value verb = verbs[i];
        if (verb.isString()) {
            SkASSERT(!strcmp(verb.asCString(), SKDEBUGCANVAS_VERB_CLOSE));
            result->close();
        }
        else {
            if (verb.isMember(SKDEBUGCANVAS_VERB_MOVE)) {
                Json::Value move = verb[SKDEBUGCANVAS_VERB_MOVE];
                result->moveTo(move[0].asFloat(), move[1].asFloat());
            }
            else if (verb.isMember(SKDEBUGCANVAS_VERB_LINE)) {
                Json::Value line = verb[SKDEBUGCANVAS_VERB_LINE];
                result->lineTo(line[0].asFloat(), line[1].asFloat());
            }
            else if (verb.isMember(SKDEBUGCANVAS_VERB_QUAD)) {
                Json::Value quad = verb[SKDEBUGCANVAS_VERB_QUAD];
                result->quadTo(quad[0][0].asFloat(), quad[0][1].asFloat(),
                               quad[1][0].asFloat(), quad[1][1].asFloat());
            }
            else if (verb.isMember(SKDEBUGCANVAS_VERB_CUBIC)) {
                Json::Value cubic = verb[SKDEBUGCANVAS_VERB_CUBIC];
                result->cubicTo(cubic[0][0].asFloat(), cubic[0][1].asFloat(),
                                cubic[1][0].asFloat(), cubic[1][1].asFloat(),
                                cubic[2][0].asFloat(), cubic[2][1].asFloat());
            }
            else if (verb.isMember(SKDEBUGCANVAS_VERB_CONIC)) {
                Json::Value conic = verb[SKDEBUGCANVAS_VERB_CONIC];
                result->conicTo(conic[0][0].asFloat(), conic[0][1].asFloat(),
                                conic[1][0].asFloat(), conic[1][1].asFloat(),
                                conic[2].asFloat());
            }
            else {
                SkASSERT(false);
            }
        }
    }
}

SkRegion::Op get_json_regionop(Json::Value& jsonOp) {
    const char* op = jsonOp.asCString();
    if (!strcmp(op, SKDEBUGCANVAS_REGIONOP_DIFFERENCE)) {
        return SkRegion::kDifference_Op;
    }
    else if (!strcmp(op, SKDEBUGCANVAS_REGIONOP_INTERSECT)) {
        return SkRegion::kIntersect_Op;
    }
    else if (!strcmp(op, SKDEBUGCANVAS_REGIONOP_UNION)) {
        return SkRegion::kUnion_Op;
    }
    else if (!strcmp(op, SKDEBUGCANVAS_REGIONOP_XOR)) {
        return SkRegion::kXOR_Op;
    }
    else if (!strcmp(op, SKDEBUGCANVAS_REGIONOP_REVERSE_DIFFERENCE)) {
        return SkRegion::kReverseDifference_Op;
    }
    else if (!strcmp(op, SKDEBUGCANVAS_REGIONOP_REPLACE)) {
        return SkRegion::kReplace_Op;
    }
    SkASSERT(false);
    return SkRegion::kIntersect_Op;
}


SkClipPathCommand::SkClipPathCommand(const SkPath& path, SkRegion::Op op, bool doAA)
    : INHERITED(kClipPath_OpType) {
    fPath = path;
    fOp = op;
    fDoAA = doAA;

    fInfo.push(SkObjectParser::PathToString(path));
    fInfo.push(SkObjectParser::RegionOpToString(op));
    fInfo.push(SkObjectParser::BoolToString(doAA));
}

void SkClipPathCommand::execute(SkCanvas* canvas) const {
    canvas->clipPath(fPath, fOp, fDoAA);
}

bool SkClipPathCommand::render(SkCanvas* canvas) const {
    render_path(canvas, fPath);
    return true;
}

Json::Value SkClipPathCommand::toJSON() const {
    Json::Value result = INHERITED::toJSON();
    result[SKDEBUGCANVAS_ATTRIBUTE_PATH] = make_json_path(fPath);
    result[SKDEBUGCANVAS_ATTRIBUTE_REGIONOP] = make_json_regionop(fOp);
    result[SKDEBUGCANVAS_ATTRIBUTE_ANTIALIAS] = fDoAA;
    return result;
}

SkClipPathCommand* SkClipPathCommand::fromJSON(Json::Value& command) {
    SkPath path;
    extract_json_path(command[SKDEBUGCANVAS_ATTRIBUTE_PATH], &path);
    return new SkClipPathCommand(path, get_json_regionop(command[SKDEBUGCANVAS_ATTRIBUTE_REGIONOP]), 
                                 command[SKDEBUGCANVAS_ATTRIBUTE_ANTIALIAS].asBool());
}

SkClipRegionCommand::SkClipRegionCommand(const SkRegion& region, SkRegion::Op op)
    : INHERITED(kClipRegion_OpType) {
    fRegion = region;
    fOp = op;

    fInfo.push(SkObjectParser::RegionToString(region));
    fInfo.push(SkObjectParser::RegionOpToString(op));
}

void SkClipRegionCommand::execute(SkCanvas* canvas) const {
    canvas->clipRegion(fRegion, fOp);
}

Json::Value SkClipRegionCommand::toJSON() const {
    Json::Value result = INHERITED::toJSON();
    result[SKDEBUGCANVAS_ATTRIBUTE_REGION] = make_json_region(fRegion);
    result[SKDEBUGCANVAS_ATTRIBUTE_REGIONOP] = make_json_regionop(fOp);
    return result;
}

SkClipRegionCommand* SkClipRegionCommand::fromJSON(Json::Value& command) {
    SkASSERT(false);
    return nullptr;
}

SkClipRectCommand::SkClipRectCommand(const SkRect& rect, SkRegion::Op op, bool doAA)
    : INHERITED(kClipRect_OpType) {
    fRect = rect;
    fOp = op;
    fDoAA = doAA;

    fInfo.push(SkObjectParser::RectToString(rect));
    fInfo.push(SkObjectParser::RegionOpToString(op));
    fInfo.push(SkObjectParser::BoolToString(doAA));
}

void SkClipRectCommand::execute(SkCanvas* canvas) const {
    canvas->clipRect(fRect, fOp, fDoAA);
}

Json::Value SkClipRectCommand::toJSON() const {
    Json::Value result = INHERITED::toJSON();
    result[SKDEBUGCANVAS_ATTRIBUTE_COORDS] = make_json_rect(fRect);
    result[SKDEBUGCANVAS_ATTRIBUTE_REGIONOP] = make_json_regionop(fOp);
    result[SKDEBUGCANVAS_ATTRIBUTE_ANTIALIAS] = Json::Value(fDoAA);
    return result;
}

SkClipRectCommand* SkClipRectCommand::fromJSON(Json::Value& command) {
    SkRect rect;
    extract_json_rect(command[SKDEBUGCANVAS_ATTRIBUTE_COORDS], &rect);
    return new SkClipRectCommand(rect, get_json_regionop(command[SKDEBUGCANVAS_ATTRIBUTE_REGIONOP]), 
                                 command[SKDEBUGCANVAS_ATTRIBUTE_ANTIALIAS].asBool());
}

SkClipRRectCommand::SkClipRRectCommand(const SkRRect& rrect, SkRegion::Op op, bool doAA)
    : INHERITED(kClipRRect_OpType) {
    fRRect = rrect;
    fOp = op;
    fDoAA = doAA;

    fInfo.push(SkObjectParser::RRectToString(rrect));
    fInfo.push(SkObjectParser::RegionOpToString(op));
    fInfo.push(SkObjectParser::BoolToString(doAA));
}

void SkClipRRectCommand::execute(SkCanvas* canvas) const {
    canvas->clipRRect(fRRect, fOp, fDoAA);
}

bool SkClipRRectCommand::render(SkCanvas* canvas) const {
    render_rrect(canvas, fRRect);
    return true;
}

Json::Value SkClipRRectCommand::toJSON() const {
    Json::Value result = INHERITED::toJSON();
    result[SKDEBUGCANVAS_ATTRIBUTE_COORDS] = make_json_rrect(fRRect);
    result[SKDEBUGCANVAS_ATTRIBUTE_REGIONOP] = make_json_regionop(fOp);
    result[SKDEBUGCANVAS_ATTRIBUTE_ANTIALIAS] = Json::Value(fDoAA);
    return result;
}

SkClipRRectCommand* SkClipRRectCommand::fromJSON(Json::Value& command) {
    SkRRect rrect;
    extract_json_rrect(command[SKDEBUGCANVAS_ATTRIBUTE_COORDS], &rrect);
    return new SkClipRRectCommand(rrect, 
                                  get_json_regionop(command[SKDEBUGCANVAS_ATTRIBUTE_REGIONOP]), 
                                  command[SKDEBUGCANVAS_ATTRIBUTE_ANTIALIAS].asBool());
}

SkConcatCommand::SkConcatCommand(const SkMatrix& matrix)
    : INHERITED(kConcat_OpType) {
    fMatrix = matrix;

    fInfo.push(SkObjectParser::MatrixToString(matrix));
}

void SkConcatCommand::execute(SkCanvas* canvas) const {
    canvas->concat(fMatrix);
}

Json::Value SkConcatCommand::toJSON() const {
    Json::Value result = INHERITED::toJSON();
    result[SKDEBUGCANVAS_ATTRIBUTE_MATRIX] = make_json_matrix(fMatrix);
    return result;
}

SkConcatCommand* SkConcatCommand::fromJSON(Json::Value& command) {
    SkMatrix matrix;
    extract_json_matrix(command[SKDEBUGCANVAS_ATTRIBUTE_MATRIX], &matrix);
    return new SkConcatCommand(matrix);
}

SkDrawBitmapCommand::SkDrawBitmapCommand(const SkBitmap& bitmap, SkScalar left, SkScalar top,
                                         const SkPaint* paint)
    : INHERITED(kDrawBitmap_OpType) {
    fBitmap = bitmap;
    fLeft = left;
    fTop = top;
    if (paint) {
        fPaint = *paint;
        fPaintPtr = &fPaint;
    } else {
        fPaintPtr = nullptr;
    }

    fInfo.push(SkObjectParser::BitmapToString(bitmap));
    fInfo.push(SkObjectParser::ScalarToString(left, "SkScalar left: "));
    fInfo.push(SkObjectParser::ScalarToString(top, "SkScalar top: "));
    if (paint) {
        fInfo.push(SkObjectParser::PaintToString(*paint));
    }
}

void SkDrawBitmapCommand::execute(SkCanvas* canvas) const {
    canvas->drawBitmap(fBitmap, fLeft, fTop, fPaintPtr);
}

bool SkDrawBitmapCommand::render(SkCanvas* canvas) const {
    render_bitmap(canvas, fBitmap);
    return true;
}

Json::Value SkDrawBitmapCommand::toJSON() const {
    Json::Value result = INHERITED::toJSON();
    Json::Value encoded;
    if (flatten(fBitmap, &encoded, SKDEBUGCANVAS_SEND_BINARIES)) {
        Json::Value command(Json::objectValue);
        result[SKDEBUGCANVAS_ATTRIBUTE_BITMAP] = encoded;
        result[SKDEBUGCANVAS_ATTRIBUTE_COORDS] = make_json_point(fLeft, fTop);
        if (fPaintPtr != nullptr) {
            result[SKDEBUGCANVAS_ATTRIBUTE_PAINT] = make_json_paint(*fPaintPtr, SKDEBUGCANVAS_SEND_BINARIES);
        }
    }
    return result;
}

SkDrawBitmapCommand* SkDrawBitmapCommand::fromJSON(Json::Value& command) {
    SkBitmap* bitmap = load_bitmap(command[SKDEBUGCANVAS_ATTRIBUTE_BITMAP]);
    if (bitmap == nullptr) {
        return nullptr;
    }
    Json::Value point = command[SKDEBUGCANVAS_ATTRIBUTE_COORDS];
    SkPaint* paintPtr;
    SkPaint paint;
    if (command.isMember(SKDEBUGCANVAS_ATTRIBUTE_PAINT)) {
        extract_json_paint(command[SKDEBUGCANVAS_ATTRIBUTE_PAINT], &paint);
        paintPtr = &paint;
    }
    else {
        paintPtr = nullptr;
    }
    SkDrawBitmapCommand* result = new SkDrawBitmapCommand(*bitmap, point[0].asFloat(), 
                                                          point[1].asFloat(), paintPtr);
    delete bitmap;
    return result;
}

SkDrawBitmapNineCommand::SkDrawBitmapNineCommand(const SkBitmap& bitmap, const SkIRect& center,
                                                 const SkRect& dst, const SkPaint* paint)
    : INHERITED(kDrawBitmapNine_OpType) {
    fBitmap = bitmap;
    fCenter = center;
    fDst = dst;
    if (paint) {
        fPaint = *paint;
        fPaintPtr = &fPaint;
    } else {
        fPaintPtr = nullptr;
    }

    fInfo.push(SkObjectParser::BitmapToString(bitmap));
    fInfo.push(SkObjectParser::IRectToString(center));
    fInfo.push(SkObjectParser::RectToString(dst, "Dst: "));
    if (paint) {
        fInfo.push(SkObjectParser::PaintToString(*paint));
    }
}

void SkDrawBitmapNineCommand::execute(SkCanvas* canvas) const {
    canvas->drawBitmapNine(fBitmap, fCenter, fDst, fPaintPtr);
}

bool SkDrawBitmapNineCommand::render(SkCanvas* canvas) const {
    SkRect tmp = SkRect::Make(fCenter);
    render_bitmap(canvas, fBitmap, &tmp);
    return true;
}

Json::Value SkDrawBitmapNineCommand::toJSON() const {
    Json::Value result = INHERITED::toJSON();
    Json::Value encoded;
    if (flatten(fBitmap, &encoded, SKDEBUGCANVAS_SEND_BINARIES)) {
        result[SKDEBUGCANVAS_ATTRIBUTE_BITMAP] = encoded;
        result[SKDEBUGCANVAS_ATTRIBUTE_CENTER] = make_json_irect(fCenter);
        result[SKDEBUGCANVAS_ATTRIBUTE_DST] = make_json_rect(fDst);
        if (fPaintPtr != nullptr) {
            result[SKDEBUGCANVAS_ATTRIBUTE_PAINT] = make_json_paint(*fPaintPtr,
                                                                    SKDEBUGCANVAS_SEND_BINARIES);
        }
    }
    return result;
}

SkDrawBitmapNineCommand* SkDrawBitmapNineCommand::fromJSON(Json::Value& command) {
    SkBitmap* bitmap = load_bitmap(command[SKDEBUGCANVAS_ATTRIBUTE_BITMAP]);
    if (bitmap == nullptr) {
        return nullptr;
    }
    SkIRect center;
    extract_json_irect(command[SKDEBUGCANVAS_ATTRIBUTE_CENTER], &center);
    SkRect dst;
    extract_json_rect(command[SKDEBUGCANVAS_ATTRIBUTE_DST], &dst);
    SkPaint* paintPtr;
    SkPaint paint;
    if (command.isMember(SKDEBUGCANVAS_ATTRIBUTE_PAINT)) {
        extract_json_paint(command[SKDEBUGCANVAS_ATTRIBUTE_PAINT], &paint);
        paintPtr = &paint;
    }
    else {
        paintPtr = nullptr;
    }
    SkDrawBitmapNineCommand* result = new SkDrawBitmapNineCommand(*bitmap, center, dst, paintPtr);
    delete bitmap;
    return result;
}

SkDrawBitmapRectCommand::SkDrawBitmapRectCommand(const SkBitmap& bitmap, const SkRect* src,
                                                 const SkRect& dst, const SkPaint* paint,
                                                 SkCanvas::SrcRectConstraint constraint)
    : INHERITED(kDrawBitmapRect_OpType) {
    fBitmap = bitmap;
    if (src) {
        fSrc = *src;
    } else {
        fSrc.setEmpty();
    }
    fDst = dst;

    if (paint) {
        fPaint = *paint;
        fPaintPtr = &fPaint;
    } else {
        fPaintPtr = nullptr;
    }
    fConstraint = constraint;

    fInfo.push(SkObjectParser::BitmapToString(bitmap));
    if (src) {
        fInfo.push(SkObjectParser::RectToString(*src, "Src: "));
    }
    fInfo.push(SkObjectParser::RectToString(dst, "Dst: "));
    if (paint) {
        fInfo.push(SkObjectParser::PaintToString(*paint));
    }
    fInfo.push(SkObjectParser::IntToString(fConstraint, "Constraint: "));
}

void SkDrawBitmapRectCommand::execute(SkCanvas* canvas) const {
    canvas->legacy_drawBitmapRect(fBitmap, this->srcRect(), fDst, fPaintPtr, fConstraint);
}

bool SkDrawBitmapRectCommand::render(SkCanvas* canvas) const {
    render_bitmap(canvas, fBitmap, this->srcRect());
    return true;
}

Json::Value SkDrawBitmapRectCommand::toJSON() const {
    Json::Value result = INHERITED::toJSON();
    Json::Value encoded;
    if (flatten(fBitmap, &encoded, SKDEBUGCANVAS_SEND_BINARIES)) {
        result[SKDEBUGCANVAS_ATTRIBUTE_BITMAP] = encoded;
        if (!fSrc.isEmpty()) {
            result[SKDEBUGCANVAS_ATTRIBUTE_SRC] = make_json_rect(fSrc);
        }
        result[SKDEBUGCANVAS_ATTRIBUTE_DST] = make_json_rect(fDst);
        if (fPaintPtr != nullptr) {
            result[SKDEBUGCANVAS_ATTRIBUTE_PAINT] = make_json_paint(*fPaintPtr,
                                                                    SKDEBUGCANVAS_SEND_BINARIES);
        }
        if (fConstraint == SkCanvas::kStrict_SrcRectConstraint) {
            result[SKDEBUGCANVAS_ATTRIBUTE_STRICT] = Json::Value(true);
        }
    }
    return result;
}

SkDrawBitmapRectCommand* SkDrawBitmapRectCommand::fromJSON(Json::Value& command) {
    SkBitmap* bitmap = load_bitmap(command[SKDEBUGCANVAS_ATTRIBUTE_BITMAP]);
    if (bitmap == nullptr) {
        return nullptr;
    }
    SkRect dst;
    extract_json_rect(command[SKDEBUGCANVAS_ATTRIBUTE_DST], &dst);
    SkPaint* paintPtr;
    SkPaint paint;
    if (command.isMember(SKDEBUGCANVAS_ATTRIBUTE_PAINT)) {
        extract_json_paint(command[SKDEBUGCANVAS_ATTRIBUTE_PAINT], &paint);
        paintPtr = &paint;
    }
    else {
        paintPtr = nullptr;
    }
    SkCanvas::SrcRectConstraint constraint;
    if (command.isMember(SKDEBUGCANVAS_ATTRIBUTE_STRICT) && 
        command[SKDEBUGCANVAS_ATTRIBUTE_STRICT].asBool()) {
        constraint = SkCanvas::kStrict_SrcRectConstraint;
    }
    else {
        constraint = SkCanvas::kFast_SrcRectConstraint;
    }
    SkRect* srcPtr;
    SkRect src;
    if (command.isMember(SKDEBUGCANVAS_ATTRIBUTE_SRC)) {
        extract_json_rect(command[SKDEBUGCANVAS_ATTRIBUTE_SRC], &src);
        srcPtr = &src;
    }
    else {
        srcPtr = nullptr;
    }
    SkDrawBitmapRectCommand* result = new SkDrawBitmapRectCommand(*bitmap, srcPtr, dst, paintPtr,
                                                                  constraint);
    delete bitmap;
    return result;
}

SkDrawImageCommand::SkDrawImageCommand(const SkImage* image, SkScalar left, SkScalar top,
                                       const SkPaint* paint)
    : INHERITED(kDrawImage_OpType)
    , fImage(SkRef(image))
    , fLeft(left)
    , fTop(top) {

    fInfo.push(SkObjectParser::ImageToString(image));
    fInfo.push(SkObjectParser::ScalarToString(left, "Left: "));
    fInfo.push(SkObjectParser::ScalarToString(top, "Top: "));

    if (paint) {
        fPaint.set(*paint);
        fInfo.push(SkObjectParser::PaintToString(*paint));
    }
}

void SkDrawImageCommand::execute(SkCanvas* canvas) const {
    canvas->drawImage(fImage, fLeft, fTop, fPaint.getMaybeNull());
}

bool SkDrawImageCommand::render(SkCanvas* canvas) const {
    SkAutoCanvasRestore acr(canvas, true);
    canvas->clear(0xFFFFFFFF);

    xlate_and_scale_to_bounds(canvas, SkRect::MakeXYWH(fLeft, fTop,
                                                       SkIntToScalar(fImage->width()),
                                                       SkIntToScalar(fImage->height())));
    this->execute(canvas);
    return true;
}

Json::Value SkDrawImageCommand::toJSON() const {
    Json::Value result = INHERITED::toJSON();
    Json::Value encoded;
    if (flatten(*fImage, &encoded, SKDEBUGCANVAS_SEND_BINARIES)) {
        result[SKDEBUGCANVAS_ATTRIBUTE_IMAGE] = encoded;
        result[SKDEBUGCANVAS_ATTRIBUTE_COORDS] = make_json_point(fLeft, fTop);
        if (fPaint.isValid()) {
            result[SKDEBUGCANVAS_ATTRIBUTE_PAINT] = make_json_paint(*fPaint.get(),
                                                                    SKDEBUGCANVAS_SEND_BINARIES);
        }
    }
    return result;
}

SkDrawImageCommand* SkDrawImageCommand::fromJSON(Json::Value& command) {
    SkImage* image = load_image(command[SKDEBUGCANVAS_ATTRIBUTE_IMAGE]);
    if (image == nullptr) {
        return nullptr;
    }
    Json::Value point = command[SKDEBUGCANVAS_ATTRIBUTE_COORDS];
    SkPaint* paintPtr;
    SkPaint paint;
    if (command.isMember(SKDEBUGCANVAS_ATTRIBUTE_PAINT)) {
        extract_json_paint(command[SKDEBUGCANVAS_ATTRIBUTE_PAINT], &paint);
        paintPtr = &paint;
    }
    else {
        paintPtr = nullptr;
    }
    SkDrawImageCommand* result = new SkDrawImageCommand(image, point[0].asFloat(), 
                                                        point[1].asFloat(), paintPtr);
    image->unref();
    return result;
}

SkDrawImageRectCommand::SkDrawImageRectCommand(const SkImage* image, const SkRect* src,
                                               const SkRect& dst, const SkPaint* paint,
                                               SkCanvas::SrcRectConstraint constraint)
    : INHERITED(kDrawImageRect_OpType)
    , fImage(SkRef(image))
    , fDst(dst)
    , fConstraint(constraint) {

    if (src) {
        fSrc.set(*src);
    }

    if (paint) {
        fPaint.set(*paint);
    }

    fInfo.push(SkObjectParser::ImageToString(image));
    if (src) {
        fInfo.push(SkObjectParser::RectToString(*src, "Src: "));
    }
    fInfo.push(SkObjectParser::RectToString(dst, "Dst: "));
    if (paint) {
        fInfo.push(SkObjectParser::PaintToString(*paint));
    }
    fInfo.push(SkObjectParser::IntToString(fConstraint, "Constraint: "));
}

void SkDrawImageRectCommand::execute(SkCanvas* canvas) const {
    canvas->legacy_drawImageRect(fImage, fSrc.getMaybeNull(), fDst, fPaint.getMaybeNull(), fConstraint);
}

bool SkDrawImageRectCommand::render(SkCanvas* canvas) const {
    SkAutoCanvasRestore acr(canvas, true);
    canvas->clear(0xFFFFFFFF);

    xlate_and_scale_to_bounds(canvas, fDst);

    this->execute(canvas);
    return true;
}

Json::Value SkDrawImageRectCommand::toJSON() const {
    Json::Value result = INHERITED::toJSON();
    Json::Value encoded;
    if (flatten(*fImage.get(), &encoded, SKDEBUGCANVAS_SEND_BINARIES)) {
        result[SKDEBUGCANVAS_ATTRIBUTE_BITMAP] = encoded;
        if (fSrc.isValid()) {
            result[SKDEBUGCANVAS_ATTRIBUTE_SRC] = make_json_rect(*fSrc.get());
        }
        result[SKDEBUGCANVAS_ATTRIBUTE_DST] = make_json_rect(fDst);
        if (fPaint.isValid()) {
            result[SKDEBUGCANVAS_ATTRIBUTE_PAINT] = make_json_paint(*fPaint.get(),
                                                                    SKDEBUGCANVAS_SEND_BINARIES);
        }
        if (fConstraint == SkCanvas::kStrict_SrcRectConstraint) {
            result[SKDEBUGCANVAS_ATTRIBUTE_STRICT] = Json::Value(true);
        }
    }
    return result;
}

SkDrawImageRectCommand* SkDrawImageRectCommand::fromJSON(Json::Value& command) {
    SkImage* image = load_image(command[SKDEBUGCANVAS_ATTRIBUTE_IMAGE]);
    if (image == nullptr) {
        return nullptr;
    }
    SkRect dst;
    extract_json_rect(command[SKDEBUGCANVAS_ATTRIBUTE_DST], &dst);
    SkPaint* paintPtr;
    SkPaint paint;
    if (command.isMember(SKDEBUGCANVAS_ATTRIBUTE_PAINT)) {
        extract_json_paint(command[SKDEBUGCANVAS_ATTRIBUTE_PAINT], &paint);
        paintPtr = &paint;
    }
    else {
        paintPtr = nullptr;
    }
    SkCanvas::SrcRectConstraint constraint;
    if (command.isMember(SKDEBUGCANVAS_ATTRIBUTE_STRICT) && 
        command[SKDEBUGCANVAS_ATTRIBUTE_STRICT].asBool()) {
        constraint = SkCanvas::kStrict_SrcRectConstraint;
    }
    else {
        constraint = SkCanvas::kFast_SrcRectConstraint;
    }
    SkRect* srcPtr;
    SkRect src;
    if (command.isMember(SKDEBUGCANVAS_ATTRIBUTE_SRC)) {
        extract_json_rect(command[SKDEBUGCANVAS_ATTRIBUTE_SRC], &src);
        srcPtr = &src;
    }
    else {
        srcPtr = nullptr;
    }
    SkDrawImageRectCommand* result = new SkDrawImageRectCommand(image, srcPtr, dst, paintPtr, 
                                                                constraint);
    image->unref();
    return result;
}

SkDrawOvalCommand::SkDrawOvalCommand(const SkRect& oval, const SkPaint& paint)
    : INHERITED(kDrawOval_OpType) {
    fOval = oval;
    fPaint = paint;

    fInfo.push(SkObjectParser::RectToString(oval));
    fInfo.push(SkObjectParser::PaintToString(paint));
}

void SkDrawOvalCommand::execute(SkCanvas* canvas) const {
    canvas->drawOval(fOval, fPaint);
}

bool SkDrawOvalCommand::render(SkCanvas* canvas) const {
    canvas->clear(0xFFFFFFFF);
    canvas->save();

    xlate_and_scale_to_bounds(canvas, fOval);

    SkPaint p;
    p.setColor(SK_ColorBLACK);
    p.setStyle(SkPaint::kStroke_Style);

    canvas->drawOval(fOval, p);
    canvas->restore();

    return true;
}

Json::Value SkDrawOvalCommand::toJSON() const {
    Json::Value result = INHERITED::toJSON();
    result[SKDEBUGCANVAS_ATTRIBUTE_COORDS] = make_json_rect(fOval);
    result[SKDEBUGCANVAS_ATTRIBUTE_PAINT] = make_json_paint(fPaint, SKDEBUGCANVAS_SEND_BINARIES);
    return result;
}

SkDrawOvalCommand* SkDrawOvalCommand::fromJSON(Json::Value& command) {
    SkRect coords;
    extract_json_rect(command[SKDEBUGCANVAS_ATTRIBUTE_COORDS], &coords);
    SkPaint paint;
    extract_json_paint(command[SKDEBUGCANVAS_ATTRIBUTE_PAINT], &paint);
    return new SkDrawOvalCommand(coords, paint);
}

SkDrawPaintCommand::SkDrawPaintCommand(const SkPaint& paint)
    : INHERITED(kDrawPaint_OpType) {
    fPaint = paint;

    fInfo.push(SkObjectParser::PaintToString(paint));
}

void SkDrawPaintCommand::execute(SkCanvas* canvas) const {
    canvas->drawPaint(fPaint);
}

bool SkDrawPaintCommand::render(SkCanvas* canvas) const {
    canvas->clear(0xFFFFFFFF);
    canvas->drawPaint(fPaint);
    return true;
}

Json::Value SkDrawPaintCommand::toJSON() const {
    Json::Value result = INHERITED::toJSON();
    result[SKDEBUGCANVAS_ATTRIBUTE_PAINT] = make_json_paint(fPaint, SKDEBUGCANVAS_SEND_BINARIES);
    return result;
}

SkDrawPaintCommand* SkDrawPaintCommand::fromJSON(Json::Value& command) {
    SkPaint paint;
    extract_json_paint(command[SKDEBUGCANVAS_ATTRIBUTE_PAINT], &paint);
    return new SkDrawPaintCommand(paint);
}

SkDrawPathCommand::SkDrawPathCommand(const SkPath& path, const SkPaint& paint)
    : INHERITED(kDrawPath_OpType) {
    fPath = path;
    fPaint = paint;

    fInfo.push(SkObjectParser::PathToString(path));
    fInfo.push(SkObjectParser::PaintToString(paint));
}

void SkDrawPathCommand::execute(SkCanvas* canvas) const {
    canvas->drawPath(fPath, fPaint);
}

bool SkDrawPathCommand::render(SkCanvas* canvas) const {
    render_path(canvas, fPath);
    return true;
}

Json::Value SkDrawPathCommand::toJSON() const {
    Json::Value result = INHERITED::toJSON();
    result[SKDEBUGCANVAS_ATTRIBUTE_PATH] = make_json_path(fPath);
    result[SKDEBUGCANVAS_ATTRIBUTE_PAINT] = make_json_paint(fPaint, SKDEBUGCANVAS_SEND_BINARIES);
    return result;
}

SkDrawPathCommand* SkDrawPathCommand::fromJSON(Json::Value& command) {
    SkPath path;
    extract_json_path(command[SKDEBUGCANVAS_ATTRIBUTE_PATH], &path);
    SkPaint paint;
    extract_json_paint(command[SKDEBUGCANVAS_ATTRIBUTE_PAINT], &paint);
    return new SkDrawPathCommand(path, paint);
}

SkBeginDrawPictureCommand::SkBeginDrawPictureCommand(const SkPicture* picture,
                                                     const SkMatrix* matrix,
                                                     const SkPaint* paint)
    : INHERITED(kBeginDrawPicture_OpType)
    , fPicture(SkRef(picture)) {

    SkString* str = new SkString;
    str->appendf("SkPicture: L: %f T: %f R: %f B: %f",
                 picture->cullRect().fLeft, picture->cullRect().fTop,
                 picture->cullRect().fRight, picture->cullRect().fBottom);
    fInfo.push(str);

    if (matrix) {
        fMatrix.set(*matrix);
        fInfo.push(SkObjectParser::MatrixToString(*matrix));
    }

    if (paint) {
        fPaint.set(*paint);
        fInfo.push(SkObjectParser::PaintToString(*paint));
    }

}

void SkBeginDrawPictureCommand::execute(SkCanvas* canvas) const {
    if (fPaint.isValid()) {
        SkRect bounds = fPicture->cullRect();
        if (fMatrix.isValid()) {
            fMatrix.get()->mapRect(&bounds);
        }
        canvas->saveLayer(&bounds, fPaint.get());
    }

    if (fMatrix.isValid()) {
        if (!fPaint.isValid()) {
            canvas->save();
        }
        canvas->concat(*fMatrix.get());
    }
}

bool SkBeginDrawPictureCommand::render(SkCanvas* canvas) const {
    canvas->clear(0xFFFFFFFF);
    canvas->save();

    xlate_and_scale_to_bounds(canvas, fPicture->cullRect());

    canvas->drawPicture(fPicture.get());

    canvas->restore();

    return true;
}

SkEndDrawPictureCommand::SkEndDrawPictureCommand(bool restore)
    : INHERITED(kEndDrawPicture_OpType) , fRestore(restore) { }

void SkEndDrawPictureCommand::execute(SkCanvas* canvas) const {
    if (fRestore) {
        canvas->restore();
    }
}

SkDrawPointsCommand::SkDrawPointsCommand(SkCanvas::PointMode mode, size_t count,
                                         const SkPoint pts[], const SkPaint& paint)
    : INHERITED(kDrawPoints_OpType) {
    fMode = mode;
    fCount = count;
    fPts = new SkPoint[count];
    memcpy(fPts, pts, count * sizeof(SkPoint));
    fPaint = paint;

    fInfo.push(SkObjectParser::PointsToString(pts, count));
    fInfo.push(SkObjectParser::ScalarToString(SkIntToScalar((unsigned int)count),
                                              "Points: "));
    fInfo.push(SkObjectParser::PointModeToString(mode));
    fInfo.push(SkObjectParser::PaintToString(paint));
}

void SkDrawPointsCommand::execute(SkCanvas* canvas) const {
    canvas->drawPoints(fMode, fCount, fPts, fPaint);
}

bool SkDrawPointsCommand::render(SkCanvas* canvas) const {
    canvas->clear(0xFFFFFFFF);
    canvas->save();

    SkRect bounds;

    bounds.setEmpty();
    for (unsigned int i = 0; i < fCount; ++i) {
        bounds.growToInclude(fPts[i].fX, fPts[i].fY);
    }

    xlate_and_scale_to_bounds(canvas, bounds);

    SkPaint p;
    p.setColor(SK_ColorBLACK);
    p.setStyle(SkPaint::kStroke_Style);

    canvas->drawPoints(fMode, fCount, fPts, p);
    canvas->restore();

    return true;
}

Json::Value SkDrawPointsCommand::toJSON() const {
    Json::Value result = INHERITED::toJSON();
    result[SKDEBUGCANVAS_ATTRIBUTE_MODE] = make_json_pointmode(fMode);
    Json::Value points(Json::arrayValue);
    for (size_t i = 0; i < fCount; i++) {
        points.append(make_json_point(fPts[i]));
    }
    result[SKDEBUGCANVAS_ATTRIBUTE_POINTS] = points;
    result[SKDEBUGCANVAS_ATTRIBUTE_PAINT] = make_json_paint(fPaint, SKDEBUGCANVAS_SEND_BINARIES);
    return result;
}

SkDrawPointsCommand* SkDrawPointsCommand::fromJSON(Json::Value& command) {
    SkCanvas::PointMode mode;
    const char* jsonMode = command[SKDEBUGCANVAS_ATTRIBUTE_MODE].asCString();
    if (!strcmp(jsonMode, SKDEBUGCANVAS_POINTMODE_POINTS)) {
        mode = SkCanvas::kPoints_PointMode;
    }
    else if (!strcmp(jsonMode, SKDEBUGCANVAS_POINTMODE_LINES)) {
        mode = SkCanvas::kLines_PointMode;
    }
    else if (!strcmp(jsonMode, SKDEBUGCANVAS_POINTMODE_POLYGON)) {
        mode = SkCanvas::kPolygon_PointMode;
    }
    else {
        SkASSERT(false);
        return nullptr;
    }
    Json::Value jsonPoints = command[SKDEBUGCANVAS_ATTRIBUTE_POINTS];
    int count = (int) jsonPoints.size();
    SkPoint* points = (SkPoint*) sk_malloc_throw(count * sizeof(SkPoint));
    for (int i = 0; i < count; i++) {
        points[i] = SkPoint::Make(jsonPoints[i][0].asFloat(), jsonPoints[i][1].asFloat());
    }
    SkPaint paint;
    extract_json_paint(command[SKDEBUGCANVAS_ATTRIBUTE_PAINT], &paint);
    SkDrawPointsCommand* result = new SkDrawPointsCommand(mode, count, points, paint);
    free(points);
    return result;
}

SkDrawPosTextCommand::SkDrawPosTextCommand(const void* text, size_t byteLength,
                                           const SkPoint pos[], const SkPaint& paint)
    : INHERITED(kDrawPosText_OpType) {
    size_t numPts = paint.countText(text, byteLength);

    fText = new char[byteLength];
    memcpy(fText, text, byteLength);
    fByteLength = byteLength;

    fPos = new SkPoint[numPts];
    memcpy(fPos, pos, numPts * sizeof(SkPoint));

    fPaint = paint;

    fInfo.push(SkObjectParser::TextToString(text, byteLength, paint.getTextEncoding()));
    // TODO(chudy): Test that this works.
    fInfo.push(SkObjectParser::PointsToString(pos, 1));
    fInfo.push(SkObjectParser::PaintToString(paint));
}

void SkDrawPosTextCommand::execute(SkCanvas* canvas) const {
    canvas->drawPosText(fText, fByteLength, fPos, fPaint);
}

Json::Value SkDrawPosTextCommand::toJSON() const {
    Json::Value result = INHERITED::toJSON();
    result[SKDEBUGCANVAS_ATTRIBUTE_TEXT] = Json::Value((const char*) fText, 
                                                       ((const char*) fText) + fByteLength);
    Json::Value coords(Json::arrayValue);
    for (size_t i = 0; i < fByteLength; i++) {
        coords.append(make_json_point(fPos[i]));
    }
    result[SKDEBUGCANVAS_ATTRIBUTE_COORDS] = coords;
    result[SKDEBUGCANVAS_ATTRIBUTE_PAINT] = make_json_paint(fPaint, SKDEBUGCANVAS_SEND_BINARIES);
    return result;
}

SkDrawPosTextCommand* SkDrawPosTextCommand::fromJSON(Json::Value& command) {
    const char* text = command[SKDEBUGCANVAS_ATTRIBUTE_TEXT].asCString();
    SkPaint paint;
    extract_json_paint(command[SKDEBUGCANVAS_ATTRIBUTE_PAINT], &paint);
    Json::Value coords = command[SKDEBUGCANVAS_ATTRIBUTE_COORDS];
    int count = (int) coords.size();
    SkPoint* points = (SkPoint*) sk_malloc_throw(count * sizeof(SkPoint));
    for (int i = 0; i < count; i++) {
        points[i] = SkPoint::Make(coords[i][0].asFloat(), coords[i][1].asFloat());
    }
    return new SkDrawPosTextCommand(text, strlen(text), points, paint);
}

SkDrawPosTextHCommand::SkDrawPosTextHCommand(const void* text, size_t byteLength,
                                             const SkScalar xpos[], SkScalar constY,
                                             const SkPaint& paint)
    : INHERITED(kDrawPosTextH_OpType) {
    size_t numPts = paint.countText(text, byteLength);

    fText = new char[byteLength];
    memcpy(fText, text, byteLength);
    fByteLength = byteLength;

    fXpos = new SkScalar[numPts];
    memcpy(fXpos, xpos, numPts * sizeof(SkScalar));

    fConstY = constY;
    fPaint = paint;

    fInfo.push(SkObjectParser::TextToString(text, byteLength, paint.getTextEncoding()));
    fInfo.push(SkObjectParser::ScalarToString(xpos[0], "XPOS: "));
    fInfo.push(SkObjectParser::ScalarToString(constY, "SkScalar constY: "));
    fInfo.push(SkObjectParser::PaintToString(paint));
}

void SkDrawPosTextHCommand::execute(SkCanvas* canvas) const {
    canvas->drawPosTextH(fText, fByteLength, fXpos, fConstY, fPaint);
}

static const char* gPositioningLabels[] = {
    "kDefault_Positioning",
    "kHorizontal_Positioning",
    "kFull_Positioning",
};

SkDrawTextBlobCommand::SkDrawTextBlobCommand(const SkTextBlob* blob, SkScalar x, SkScalar y,
                                             const SkPaint& paint)
    : INHERITED(kDrawTextBlob_OpType)
    , fBlob(SkRef(blob))
    , fXPos(x)
    , fYPos(y)
    , fPaint(paint) {

    SkAutoTDelete<SkString> runsStr(new SkString);
    fInfo.push(SkObjectParser::ScalarToString(x, "XPOS: "));
    fInfo.push(SkObjectParser::ScalarToString(y, "YPOS: "));
    fInfo.push(SkObjectParser::RectToString(fBlob->bounds(), "Bounds: "));
    fInfo.push(runsStr);
    fInfo.push(SkObjectParser::PaintToString(paint));

    unsigned runs = 0;
    SkPaint runPaint(paint);
    SkTextBlobRunIterator iter(blob);
    while (!iter.done()) {
        SkAutoTDelete<SkString> tmpStr(new SkString);
        tmpStr->printf("==== Run [%d] ====", runs++);
        fInfo.push(tmpStr.release());

        fInfo.push(SkObjectParser::IntToString(iter.glyphCount(), "GlyphCount: "));
        tmpStr.reset(new SkString("GlyphPositioning: "));
        tmpStr->append(gPositioningLabels[iter.positioning()]);
        fInfo.push(tmpStr.release());

        iter.applyFontToPaint(&runPaint);
        fInfo.push(SkObjectParser::PaintToString(runPaint));

        iter.next();
    }

    runsStr->printf("Runs: %d", runs);
    // runStr is owned by fInfo at this point.
    runsStr.release();
}

void SkDrawTextBlobCommand::execute(SkCanvas* canvas) const {
    canvas->drawTextBlob(fBlob, fXPos, fYPos, fPaint);
}

bool SkDrawTextBlobCommand::render(SkCanvas* canvas) const {
    canvas->clear(SK_ColorWHITE);
    canvas->save();

    SkRect bounds = fBlob->bounds().makeOffset(fXPos, fYPos);
    xlate_and_scale_to_bounds(canvas, bounds);

    canvas->drawTextBlob(fBlob.get(), fXPos, fYPos, fPaint);

    canvas->restore();

    return true;
}

Json::Value SkDrawTextBlobCommand::toJSON() const {
    Json::Value result = INHERITED::toJSON();
    Json::Value runs(Json::arrayValue);
    SkTextBlobRunIterator iter(fBlob.get());
    while (!iter.done()) {
        Json::Value run(Json::objectValue);
        Json::Value jsonPositions(Json::arrayValue);
        Json::Value jsonGlyphs(Json::arrayValue);
        const SkScalar* iterPositions = iter.pos();
        const uint16_t* iterGlyphs = iter.glyphs();
        for (uint32_t i = 0; i < iter.glyphCount(); i++) {
            switch (iter.positioning()) {
                case SkTextBlob::kFull_Positioning:
                    jsonPositions.append(make_json_point(iterPositions[i * 2],
                                                         iterPositions[i * 2 + 1]));
                    break;
                case SkTextBlob::kHorizontal_Positioning:
                    jsonPositions.append(Json::Value(iterPositions[i]));
                    break;
                case SkTextBlob::kDefault_Positioning:
                    break;
            }
            jsonGlyphs.append(Json::Value(iterGlyphs[i]));
        }
        if (iter.positioning() != SkTextBlob::kDefault_Positioning) {
            run[SKDEBUGCANVAS_ATTRIBUTE_POSITIONS] = jsonPositions;
        }
        run[SKDEBUGCANVAS_ATTRIBUTE_GLYPHS] = jsonGlyphs;
        SkPaint fontPaint;
        iter.applyFontToPaint(&fontPaint);
        run[SKDEBUGCANVAS_ATTRIBUTE_FONT] = make_json_paint(fontPaint, SKDEBUGCANVAS_SEND_BINARIES);
        run[SKDEBUGCANVAS_ATTRIBUTE_COORDS] = make_json_point(iter.offset());
        runs.append(run);
        iter.next();
    }
    result[SKDEBUGCANVAS_ATTRIBUTE_RUNS] = runs;
    result[SKDEBUGCANVAS_ATTRIBUTE_X] = Json::Value(fXPos);
    result[SKDEBUGCANVAS_ATTRIBUTE_Y] = Json::Value(fYPos);
    result[SKDEBUGCANVAS_ATTRIBUTE_PAINT] = make_json_paint(fPaint, SKDEBUGCANVAS_SEND_BINARIES);
    return result;
}

SkDrawTextBlobCommand* SkDrawTextBlobCommand::fromJSON(Json::Value& command) {
    SkTextBlobBuilder builder;
    Json::Value runs = command[SKDEBUGCANVAS_ATTRIBUTE_RUNS];
    for (Json::ArrayIndex i = 0 ; i < runs.size(); i++) {
        Json::Value run = runs[i];
        SkPaint font;
        font.setTextEncoding(SkPaint::kGlyphID_TextEncoding);
        extract_json_paint(run[SKDEBUGCANVAS_ATTRIBUTE_FONT], &font);
        Json::Value glyphs = run[SKDEBUGCANVAS_ATTRIBUTE_GLYPHS];
        int count = glyphs.size();
        Json::Value coords = run[SKDEBUGCANVAS_ATTRIBUTE_COORDS];
        SkScalar x = coords[0].asFloat();
        SkScalar y = coords[1].asFloat();
        if (run.isMember(SKDEBUGCANVAS_ATTRIBUTE_POSITIONS)) {
            Json::Value positions = run[SKDEBUGCANVAS_ATTRIBUTE_POSITIONS];
            if (positions.size() > 0 && positions[0].isNumeric()) {
                SkTextBlobBuilder::RunBuffer buffer = builder.allocRunPosH(font, count, y);
                for (int j = 0; j < count; j++) {
                    buffer.glyphs[j] = glyphs[j].asUInt();
                    buffer.pos[j] = positions[j].asFloat();
                }
            }
            else {
                SkTextBlobBuilder::RunBuffer buffer = builder.allocRunPos(font, count);
                for (int j = 0; j < count; j++) {
                    buffer.glyphs[j] = glyphs[j].asUInt();
                    buffer.pos[j * 2] = positions[j][0].asFloat();
                    buffer.pos[j * 2 + 1] = positions[j][1].asFloat();
                }
            }
        }
        else {
            SkTextBlobBuilder::RunBuffer buffer = builder.allocRun(font, count, x, y);
            for (int j = 0; j < count; j++) {
                buffer.glyphs[j] = glyphs[j].asUInt();
            }
        }
    }
    SkScalar x = command[SKDEBUGCANVAS_ATTRIBUTE_X].asFloat();
    SkScalar y = command[SKDEBUGCANVAS_ATTRIBUTE_Y].asFloat();
    SkPaint paint;
    extract_json_paint(command[SKDEBUGCANVAS_ATTRIBUTE_PAINT], &paint);
    return new SkDrawTextBlobCommand(builder.build(), x, y, paint);
}

SkDrawPatchCommand::SkDrawPatchCommand(const SkPoint cubics[12], const SkColor colors[4],
                                       const SkPoint texCoords[4], SkXfermode* xfermode,
                                       const SkPaint& paint)
    : INHERITED(kDrawPatch_OpType) {
    memcpy(fCubics, cubics, sizeof(fCubics));
    memcpy(fColors, colors, sizeof(fColors));
    memcpy(fTexCoords, texCoords, sizeof(fTexCoords));
    fXfermode.reset(xfermode);
    fPaint = paint;

    fInfo.push(SkObjectParser::PaintToString(paint));
}

void SkDrawPatchCommand::execute(SkCanvas* canvas) const {
    canvas->drawPatch(fCubics, fColors, fTexCoords, fXfermode, fPaint);
}

SkDrawRectCommand::SkDrawRectCommand(const SkRect& rect, const SkPaint& paint)
    : INHERITED(kDrawRect_OpType) {
    fRect = rect;
    fPaint = paint;

    fInfo.push(SkObjectParser::RectToString(rect));
    fInfo.push(SkObjectParser::PaintToString(paint));
}

void SkDrawRectCommand::execute(SkCanvas* canvas) const {
    canvas->drawRect(fRect, fPaint);
}

Json::Value SkDrawRectCommand::toJSON() const {
    Json::Value result = INHERITED::toJSON();
    result[SKDEBUGCANVAS_ATTRIBUTE_COORDS] = make_json_rect(fRect);
    result[SKDEBUGCANVAS_ATTRIBUTE_PAINT] = make_json_paint(fPaint, SKDEBUGCANVAS_SEND_BINARIES);
    return result;
}

SkDrawRectCommand* SkDrawRectCommand::fromJSON(Json::Value& command) {
    SkRect coords;
    extract_json_rect(command[SKDEBUGCANVAS_ATTRIBUTE_COORDS], &coords);
    SkPaint paint;
    extract_json_paint(command[SKDEBUGCANVAS_ATTRIBUTE_PAINT], &paint);
    return new SkDrawRectCommand(coords, paint);
}

SkDrawRRectCommand::SkDrawRRectCommand(const SkRRect& rrect, const SkPaint& paint)
    : INHERITED(kDrawRRect_OpType) {
    fRRect = rrect;
    fPaint = paint;

    fInfo.push(SkObjectParser::RRectToString(rrect));
    fInfo.push(SkObjectParser::PaintToString(paint));
}

void SkDrawRRectCommand::execute(SkCanvas* canvas) const {
    canvas->drawRRect(fRRect, fPaint);
}

bool SkDrawRRectCommand::render(SkCanvas* canvas) const {
    render_rrect(canvas, fRRect);
    return true;
}

Json::Value SkDrawRRectCommand::toJSON() const {
    Json::Value result = INHERITED::toJSON();
    result[SKDEBUGCANVAS_ATTRIBUTE_COORDS] = make_json_rrect(fRRect);
    result[SKDEBUGCANVAS_ATTRIBUTE_PAINT] = make_json_paint(fPaint, SKDEBUGCANVAS_SEND_BINARIES);
    return result;
}

SkDrawRRectCommand* SkDrawRRectCommand::fromJSON(Json::Value& command) {
    SkRRect coords;
    extract_json_rrect(command[SKDEBUGCANVAS_ATTRIBUTE_COORDS], &coords);
    SkPaint paint;
    extract_json_paint(command[SKDEBUGCANVAS_ATTRIBUTE_PAINT], &paint);
    return new SkDrawRRectCommand(coords, paint);
}

SkDrawDRRectCommand::SkDrawDRRectCommand(const SkRRect& outer,
                                         const SkRRect& inner,
                                         const SkPaint& paint)
    : INHERITED(kDrawDRRect_OpType) {
    fOuter = outer;
    fInner = inner;
    fPaint = paint;

    fInfo.push(SkObjectParser::RRectToString(outer));
    fInfo.push(SkObjectParser::RRectToString(inner));
    fInfo.push(SkObjectParser::PaintToString(paint));
}

void SkDrawDRRectCommand::execute(SkCanvas* canvas) const {
    canvas->drawDRRect(fOuter, fInner, fPaint);
}

bool SkDrawDRRectCommand::render(SkCanvas* canvas) const {
    render_drrect(canvas, fOuter, fInner);
    return true;
}

Json::Value SkDrawDRRectCommand::toJSON() const {
    Json::Value result = INHERITED::toJSON();
    result[SKDEBUGCANVAS_ATTRIBUTE_OUTER] = make_json_rrect(fOuter);
    result[SKDEBUGCANVAS_ATTRIBUTE_INNER] = make_json_rrect(fInner);
    result[SKDEBUGCANVAS_ATTRIBUTE_PAINT] = make_json_paint(fPaint, SKDEBUGCANVAS_SEND_BINARIES);
    return result;
}

SkDrawDRRectCommand* SkDrawDRRectCommand::fromJSON(Json::Value& command) {
    SkRRect outer;
    extract_json_rrect(command[SKDEBUGCANVAS_ATTRIBUTE_INNER], &outer);
    SkRRect inner;
    extract_json_rrect(command[SKDEBUGCANVAS_ATTRIBUTE_INNER], &inner);
    SkPaint paint;
    extract_json_paint(command[SKDEBUGCANVAS_ATTRIBUTE_PAINT], &paint);
    return new SkDrawDRRectCommand(outer, inner, paint);
}

SkDrawTextCommand::SkDrawTextCommand(const void* text, size_t byteLength, SkScalar x, SkScalar y,
                                     const SkPaint& paint)
    : INHERITED(kDrawText_OpType) {
    fText = new char[byteLength];
    memcpy(fText, text, byteLength);
    fByteLength = byteLength;
    fX = x;
    fY = y;
    fPaint = paint;

    fInfo.push(SkObjectParser::TextToString(text, byteLength, paint.getTextEncoding()));
    fInfo.push(SkObjectParser::ScalarToString(x, "SkScalar x: "));
    fInfo.push(SkObjectParser::ScalarToString(y, "SkScalar y: "));
    fInfo.push(SkObjectParser::PaintToString(paint));
}

void SkDrawTextCommand::execute(SkCanvas* canvas) const {
    canvas->drawText(fText, fByteLength, fX, fY, fPaint);
}

Json::Value SkDrawTextCommand::toJSON() const {
    Json::Value result = INHERITED::toJSON();
    result[SKDEBUGCANVAS_ATTRIBUTE_TEXT] = Json::Value((const char*) fText, 
                                                       ((const char*) fText) + fByteLength);
    Json::Value coords(Json::arrayValue);
    result[SKDEBUGCANVAS_ATTRIBUTE_COORDS] = make_json_point(fX, fY);
    result[SKDEBUGCANVAS_ATTRIBUTE_PAINT] = make_json_paint(fPaint, SKDEBUGCANVAS_SEND_BINARIES);
    return result;
}

SkDrawTextCommand* SkDrawTextCommand::fromJSON(Json::Value& command) {
    const char* text = command[SKDEBUGCANVAS_ATTRIBUTE_TEXT].asCString();
    SkPaint paint;
    extract_json_paint(command[SKDEBUGCANVAS_ATTRIBUTE_PAINT], &paint);
    Json::Value coords = command[SKDEBUGCANVAS_ATTRIBUTE_COORDS];
    return new SkDrawTextCommand(text, strlen(text), coords[0].asFloat(), coords[1].asFloat(), 
                                 paint);
}

SkDrawTextOnPathCommand::SkDrawTextOnPathCommand(const void* text, size_t byteLength,
                                                 const SkPath& path, const SkMatrix* matrix,
                                                 const SkPaint& paint)
    : INHERITED(kDrawTextOnPath_OpType) {
    fText = new char[byteLength];
    memcpy(fText, text, byteLength);
    fByteLength = byteLength;
    fPath = path;
    if (matrix) {
        fMatrix = *matrix;
    } else {
        fMatrix.setIdentity();
    }
    fPaint = paint;

    fInfo.push(SkObjectParser::TextToString(text, byteLength, paint.getTextEncoding()));
    fInfo.push(SkObjectParser::PathToString(path));
    if (matrix) {
        fInfo.push(SkObjectParser::MatrixToString(*matrix));
    }
    fInfo.push(SkObjectParser::PaintToString(paint));
}

void SkDrawTextOnPathCommand::execute(SkCanvas* canvas) const {
    canvas->drawTextOnPath(fText, fByteLength, fPath,
                           fMatrix.isIdentity() ? nullptr : &fMatrix,
                           fPaint);
}

Json::Value SkDrawTextOnPathCommand::toJSON() const {
    Json::Value result = INHERITED::toJSON();
    result[SKDEBUGCANVAS_ATTRIBUTE_TEXT] = Json::Value((const char*) fText, 
                                                       ((const char*) fText) + fByteLength);
    Json::Value coords(Json::arrayValue);
    result[SKDEBUGCANVAS_ATTRIBUTE_PATH] = make_json_path(fPath);
    if (!fMatrix.isIdentity()) {
        result[SKDEBUGCANVAS_ATTRIBUTE_MATRIX] = make_json_matrix(fMatrix);
    }
    result[SKDEBUGCANVAS_ATTRIBUTE_PAINT] = make_json_paint(fPaint, SKDEBUGCANVAS_SEND_BINARIES);
    return result;
}

SkDrawTextOnPathCommand* SkDrawTextOnPathCommand::fromJSON(Json::Value& command) {
    const char* text = command[SKDEBUGCANVAS_ATTRIBUTE_TEXT].asCString();
    SkPaint paint;
    extract_json_paint(command[SKDEBUGCANVAS_ATTRIBUTE_PAINT], &paint);
    SkPath path;
    extract_json_path(command[SKDEBUGCANVAS_ATTRIBUTE_PATH], &path);
    SkMatrix* matrixPtr;
    SkMatrix matrix;
    if (command.isMember(SKDEBUGCANVAS_ATTRIBUTE_MATRIX)) {
        extract_json_matrix(command[SKDEBUGCANVAS_ATTRIBUTE_MATRIX], &matrix);
        matrixPtr = &matrix;
    }
    else {
        matrixPtr = nullptr;
    }
    return new SkDrawTextOnPathCommand(text, strlen(text), path, matrixPtr, paint);
}

SkDrawVerticesCommand::SkDrawVerticesCommand(SkCanvas::VertexMode vmode, int vertexCount,
                                             const SkPoint vertices[], const SkPoint texs[],
                                             const SkColor colors[], SkXfermode* xfermode,
                                             const uint16_t indices[], int indexCount,
                                             const SkPaint& paint)
    : INHERITED(kDrawVertices_OpType) {
    fVmode = vmode;

    fVertexCount = vertexCount;

    fVertices = new SkPoint[vertexCount];
    memcpy(fVertices, vertices, vertexCount * sizeof(SkPoint));

    if (texs) {
        fTexs = new SkPoint[vertexCount];
        memcpy(fTexs, texs, vertexCount * sizeof(SkPoint));
    } else {
        fTexs = nullptr;
    }

    if (colors) {
        fColors = new SkColor[vertexCount];
        memcpy(fColors, colors, vertexCount * sizeof(SkColor));
    } else {
        fColors = nullptr;
    }

    fXfermode = xfermode;
    if (fXfermode) {
        fXfermode->ref();
    }

    if (indexCount > 0) {
        fIndices = new uint16_t[indexCount];
        memcpy(fIndices, indices, indexCount * sizeof(uint16_t));
    } else {
        fIndices = nullptr;
    }

    fIndexCount = indexCount;
    fPaint = paint;

    // TODO(chudy)
    fInfo.push(SkObjectParser::CustomTextToString("To be implemented."));
    fInfo.push(SkObjectParser::PaintToString(paint));
}

SkDrawVerticesCommand::~SkDrawVerticesCommand() {
    delete [] fVertices;
    delete [] fTexs;
    delete [] fColors;
    SkSafeUnref(fXfermode);
    delete [] fIndices;
}

void SkDrawVerticesCommand::execute(SkCanvas* canvas) const {
    canvas->drawVertices(fVmode, fVertexCount, fVertices,
                         fTexs, fColors, fXfermode, fIndices,
                         fIndexCount, fPaint);
}

SkRestoreCommand::SkRestoreCommand()
    : INHERITED(kRestore_OpType) {
    fInfo.push(SkObjectParser::CustomTextToString("No Parameters"));
}

void SkRestoreCommand::execute(SkCanvas* canvas) const {
    canvas->restore();
}

SkRestoreCommand* SkRestoreCommand::fromJSON(Json::Value& command) {
    return new SkRestoreCommand();
}

SkSaveCommand::SkSaveCommand()
    : INHERITED(kSave_OpType) {
}

void SkSaveCommand::execute(SkCanvas* canvas) const {
    canvas->save();
}

SkSaveCommand* SkSaveCommand::fromJSON(Json::Value& command) {
    return new SkSaveCommand();
}

SkSaveLayerCommand::SkSaveLayerCommand(const SkCanvas::SaveLayerRec& rec)
    : INHERITED(kSaveLayer_OpType) {
    if (rec.fBounds) {
        fBounds = *rec.fBounds;
    } else {
        fBounds.setEmpty();
    }

    if (rec.fPaint) {
        fPaint = *rec.fPaint;
        fPaintPtr = &fPaint;
    } else {
        fPaintPtr = nullptr;
    }
    fSaveLayerFlags = rec.fSaveLayerFlags;

    if (rec.fBackdrop) {
        fBackdrop = rec.fBackdrop;
        fBackdrop->ref();
    } else {
        fBackdrop = nullptr;
    }

    if (rec.fBounds) {
        fInfo.push(SkObjectParser::RectToString(*rec.fBounds, "Bounds: "));
    }
    if (rec.fPaint) {
        fInfo.push(SkObjectParser::PaintToString(*rec.fPaint));
    }
    fInfo.push(SkObjectParser::SaveLayerFlagsToString(fSaveLayerFlags));
}

SkSaveLayerCommand::~SkSaveLayerCommand() {
    if (fBackdrop != nullptr) {
        fBackdrop->unref();
    }
}

void SkSaveLayerCommand::execute(SkCanvas* canvas) const {
    canvas->saveLayer(SkCanvas::SaveLayerRec(fBounds.isEmpty() ? nullptr : &fBounds,
                                             fPaintPtr,
                                             fSaveLayerFlags));
}

void SkSaveLayerCommand::vizExecute(SkCanvas* canvas) const {
    canvas->save();
}

Json::Value SkSaveLayerCommand::toJSON() const {
    Json::Value result = INHERITED::toJSON();
    if (!fBounds.isEmpty()) {
        result[SKDEBUGCANVAS_ATTRIBUTE_BOUNDS] = make_json_rect(fBounds);
    }
    if (fPaintPtr != nullptr) {
        result[SKDEBUGCANVAS_ATTRIBUTE_PAINT] = make_json_paint(*fPaintPtr, 
                                                                SKDEBUGCANVAS_SEND_BINARIES);
    }
    if (fBackdrop != nullptr) {
        Json::Value jsonBackdrop;
        flatten(fBackdrop, &jsonBackdrop, SKDEBUGCANVAS_SEND_BINARIES);
        result[SKDEBUGCANVAS_ATTRIBUTE_BACKDROP] = jsonBackdrop;
    }
    if (fSaveLayerFlags != 0) {
        SkDebugf("unsupported: saveLayer flags\n");
        SkASSERT(false);
    }
    return result;
}

SkSaveLayerCommand* SkSaveLayerCommand::fromJSON(Json::Value& command) {
    SkCanvas::SaveLayerRec rec;
    SkRect bounds;
    if (command.isMember(SKDEBUGCANVAS_ATTRIBUTE_BOUNDS)) {
        extract_json_rect(command[SKDEBUGCANVAS_ATTRIBUTE_BOUNDS], &bounds);
        rec.fBounds = &bounds;
    }
    SkPaint paint;
    if (command.isMember(SKDEBUGCANVAS_ATTRIBUTE_PAINT)) {
        extract_json_paint(command[SKDEBUGCANVAS_ATTRIBUTE_PAINT], &paint);
        rec.fPaint = &paint;
    }
    if (command.isMember(SKDEBUGCANVAS_ATTRIBUTE_BACKDROP)) {
        Json::Value backdrop = command[SKDEBUGCANVAS_ATTRIBUTE_BACKDROP];
        rec.fBackdrop = (SkImageFilter*) load_flattenable(backdrop);
    }
    SkSaveLayerCommand* result = new SkSaveLayerCommand(rec);
    if (rec.fBackdrop != nullptr) {
        rec.fBackdrop->unref();
    }
    return result;
}

SkSetMatrixCommand::SkSetMatrixCommand(const SkMatrix& matrix)
    : INHERITED(kSetMatrix_OpType) {
    fUserMatrix.reset();
    fMatrix = matrix;

    fInfo.push(SkObjectParser::MatrixToString(matrix));
}

void SkSetMatrixCommand::setUserMatrix(const SkMatrix& userMatrix) {
    fUserMatrix = userMatrix;
}

void SkSetMatrixCommand::execute(SkCanvas* canvas) const {
    SkMatrix temp = SkMatrix::Concat(fUserMatrix, fMatrix);
    canvas->setMatrix(temp);
}

Json::Value SkSetMatrixCommand::toJSON() const {
    Json::Value result = INHERITED::toJSON();
    result[SKDEBUGCANVAS_ATTRIBUTE_MATRIX] = make_json_matrix(fMatrix);
    return result;
}

SkSetMatrixCommand* SkSetMatrixCommand::fromJSON(Json::Value& command) {
    SkMatrix matrix;
    extract_json_matrix(command[SKDEBUGCANVAS_ATTRIBUTE_MATRIX], &matrix);
    return new SkSetMatrixCommand(matrix);
}