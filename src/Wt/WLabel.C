/*
 * Copyright (C) 2008 Emweb bvba, Kessel-Lo, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#include "Wt/WApplication.h"
#include "Wt/WLabel.h"
#include "Wt/WText.h"
#include "Wt/WImage.h"
#include "Wt/WFormWidget.h"

#include "DomElement.h"

namespace Wt {

WLabel::WLabel()
  : buddyChanged_(false),
    newImage_(false),
    newText_(false)
{ }

WLabel::WLabel(const WString& text)
  : WLabel()
{
  manageWidget(text_, std::unique_ptr<WText>(new WText(text)));
  text_->setWordWrap(false);
}

WLabel::WLabel(std::unique_ptr<WImage> image)
  : WLabel()
{ 
  manageWidget(image_, std::move(image));
}

WLabel::~WLabel()
{
  beingDeleted();
  manageWidget(text_, std::unique_ptr<WText>());
  manageWidget(image_, std::unique_ptr<WImage>());
  setBuddy(nullptr);
}

WFormWidget *WLabel::buddy() const
{
  return buddy_.get();
}

void WLabel::setBuddy(WFormWidget *buddy)
{
  if (buddy_)
    buddy_->setLabel(nullptr);

  buddy_ = buddy;
  if (buddy_)
    buddy_->setLabel(this);

  buddyChanged_ = true;
  repaint();
}

void WLabel::setText(const WString& text)
{
  if (this->text() == text)
    return;

  if (!text_) {
    manageWidget(text_, std::unique_ptr<WText>(new WText(text)));
    text_->setWordWrap(false);
    newText_ = true;
    repaint(RepaintFlag::SizeAffected);
  }

  text_->setText(text);
}

bool WLabel::setTextFormat(TextFormat format)
{
  if (!text_) {
    setText("A"); setText("");
  }

  return text_->setTextFormat(format);
}

TextFormat WLabel::textFormat() const
{
  if (!text_)
    return TextFormat::XHTML;
  else
    return text_->textFormat();
}

WString WLabel::text() const
{
  if (text_)
    return text_->text();
  else
    return WString::Empty;
}

void WLabel::setImage(std::unique_ptr<WImage> image, Side side)
{
  manageWidget(image_, std::move(image));
  imageSide_ = side;
  newImage_ = true;
  repaint(RepaintFlag::SizeAffected);
}

void WLabel::setWordWrap(bool wordWrap)
{
 if (!text_) {
    setText("A"); setText("");
  }

  text_->setWordWrap(wordWrap);
}

bool WLabel::wordWrap() const
{
  return text_ ? text_->wordWrap() : false;
}

void WLabel::updateDom(DomElement& element, bool all)
{
  WApplication *app = WApplication::instance();

  if (image_ && text_)
    if (imageSide_ == Side::Left) {
      updateImage(element, all, app, 0);
      updateText(element, all, app, 1);
    } else {
      updateText(element, all, app, 0);
      updateImage(element, all, app, 1);
    }
  else {
    updateText(element, all, app, 0);
    updateImage(element, all, app, 0);
  }

  if (buddyChanged_ || all) {
    if (buddy_)
      element.setAttribute("for", buddy_->formName());
    buddyChanged_ = false;
  }

  WInteractWidget::updateDom(element, all);
}

void WLabel::updateImage(DomElement& element, bool all, WApplication *app,
			 int pos)
{
  if (newImage_ || all) {
    if (image_)
      element.insertChildAt(image_->createSDomElement(app), pos);
    newImage_ = false;
  }
}

void WLabel::updateText(DomElement& element, bool all, WApplication *app,
			int pos)
{
  if (newText_ || all) {
    if (text_)
      element.insertChildAt(text_->createSDomElement(app), pos);
    newText_ = false;
  }
}

void WLabel::propagateSetEnabled(bool enabled)
{
  if (text_)
    text_->propagateSetEnabled(enabled);

  WInteractWidget::propagateSetEnabled(enabled);
}

void WLabel::propagateRenderOk(bool deep)
{
  newImage_ = false;
  newText_ = false;
  buddyChanged_ = false;

  WInteractWidget::propagateRenderOk(deep);
}

DomElementType WLabel::domElementType() const
{
  // The label in <a><label /></a> eats an onclick event in IE
  // We should explicitly continue to propagate the onclick event ? 
  // For now we avoid to create a LABEL element if no buddy is set
  // (This is used e.g. in WTreeView)
  if (buddy_)
    return DomElementType::LABEL;
  else
    return isInline() ? DomElementType::SPAN : DomElementType::DIV;
}

void WLabel::getDomChanges(std::vector<DomElement *>& result,
			   WApplication *app)
{
  WInteractWidget::getDomChanges(result, app);

  if (text_)
    text_->getDomChanges(result, app);
  if (image_)
    image_->getDomChanges(result, app);
}

void WLabel::iterateChildren(const HandleWidgetMethod &method) const
{
  if (text_)
    method(text_.get());
  if (image_)
    method(image_.get());
}

}
