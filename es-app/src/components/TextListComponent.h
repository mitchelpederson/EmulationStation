#pragma once

#include "components/IList.h"
#include "Renderer.h"
#include "resources/Font.h"
#include "InputManager.h"
#include "Sound.h"
#include "Log.h"
#include "ThemeData.h"
#include "Util.h"
#include <vector>
#include <string>
#include <memory>
#include <functional>

struct TextListData
{
	unsigned int colorId;
	std::shared_ptr<TextCache> textCache;
};

//A graphical list. Supports multiple colors for rows and scrolling.
template <typename T>
class TextListComponent : public IList<TextListData, T>
{
protected:
	using IList<TextListData, T>::mEntries;
	using IList<TextListData, T>::listUpdate;
	using IList<TextListData, T>::listInput;
	using IList<TextListData, T>::listRenderTitleOverlay;
	using IList<TextListData, T>::getTransform;
	using IList<TextListData, T>::mSize;
	using IList<TextListData, T>::mCursor;
	using IList<TextListData, T>::Entry;

public:
	using IList<TextListData, T>::size;
	using IList<TextListData, T>::isScrolling;
	using IList<TextListData, T>::stopScrolling;

	TextListComponent(Window* window);
	
	bool input(InputConfig* config, Input input) override;
	void update(int deltaTime) override;
	void render(const Eigen::Affine3f& parentTrans) override;
	void applyTheme(const std::shared_ptr<ThemeData>& theme, const std::string& view, const std::string& element, unsigned int properties) override;

	void add(const std::string& name, const T& obj, unsigned int colorId);
	
	enum Alignment
	{
		ALIGN_LEFT,
		ALIGN_CENTER,
		ALIGN_RIGHT
	};

	inline void setAlignment(Alignment align) { mAlignment = align; }

	inline void setCursorChangedCallback(const std::function<void(CursorState state)>& func) { mCursorChangedCallback = func; }

	inline void setFont(const std::shared_ptr<Font>& font)
	{
		mFont = font;
		for(auto it = mEntries.begin(); it != mEntries.end(); it++)
			it->data.textCache.reset();
	}

	inline void setUppercase(bool uppercase) 
	{
		mUppercase = true;
		for(auto it = mEntries.begin(); it != mEntries.end(); it++)
			it->data.textCache.reset();
	}

	inline void setSelectorColor(unsigned int color) { mSelectorColor = color; }
	inline void setSelectedColor(unsigned int color) { mSelectedColor = color; }
	inline void setScrollSound(const std::shared_ptr<Sound>& sound) { mScrollSound = sound; }
	inline void setColor(unsigned int id, unsigned int color) { mColors[id] = color; }
	inline void setSound(const std::shared_ptr<Sound>& sound) { mScrollSound = sound; }
	inline void setLineSpacing(float lineSpacing) { mLineSpacing = lineSpacing; }

protected:
	virtual void onScroll(int amt) { if(mScrollSound) mScrollSound->play(); }
	virtual void onCursorChanged(const CursorState& state);

private:
	static const int MARQUEE_DELAY = 2000;
	static const int MARQUEE_SPEED = 8;
	static const int MARQUEE_RATE = 1;

	int mMarqueeOffset;
	int mMarqueeTime;

	Alignment mAlignment;
	float mHorizontalMargin;

	std::function<void(CursorState state)> mCursorChangedCallback;

	std::shared_ptr<Font> mFont;
	bool mUppercase;
	float mLineSpacing;
	unsigned int mSelectorColor;
	unsigned int mSelectedColor;
	std::shared_ptr<Sound> mScrollSound;
	static const unsigned int COLOR_ID_COUNT = 2;
	unsigned int mColors[COLOR_ID_COUNT];

	void drawText(int item, float xOffset, float yOffset, std::shared_ptr<Font>& font, Eigen::Affine3f& trans);

};

template <typename T>
TextListComponent<T>::TextListComponent(Window* window) : 
	IList<TextListData, T>(window)
{
	mMarqueeOffset = 0;
	mMarqueeTime = -MARQUEE_DELAY;

	mHorizontalMargin = 0;
	mAlignment = ALIGN_CENTER;

	mFont = Font::get(FONT_SIZE_MEDIUM);
	mUppercase = false;
	mLineSpacing = 1.5f;
	mSelectorColor = 0x000000FF;
	mSelectedColor = 0;
	mColors[0] = 0x0000FFFF;
	mColors[1] = 0x00FF00FF;
}

template <typename T>
void TextListComponent<T>::render(const Eigen::Affine3f& parentTrans)
{
	Eigen::Affine3f trans = parentTrans * getTransform();
	
	std::shared_ptr<Font>& font = mFont;

	if(size() == 0)
		return;

	const float entrySize = round(font->getHeight(mLineSpacing));

	int startEntry = 0;



	//number of entries that can fit on the screen simultaniously
	/*int screenCount = (int)(mSize.y() / entrySize + 0.5f);
	
	if(size() >= screenCount)
	{
		startEntry = mCursor - screenCount/2;
		if(startEntry < 0)
			startEntry = 0;
		if(startEntry >= size() - screenCount)
			startEntry = size() - screenCount;
	}*/



		// Edited by Mitchel Pederson
	int screenCount = 5;
	if (size() >= screenCount) {

		startEntry = mCursor - 2;
		if(startEntry < 0) {
			startEntry = 0;
		}

		else if (startEntry >= size() - screenCount) {
			startEntry = size() - screenCount;
		}
	}
		// End edits


	float y = 0;

	int listCutoff = startEntry + screenCount;
	if(listCutoff > size())
		listCutoff = size();

	// draw selector bar
	if(startEntry < listCutoff)
	{
		Renderer::setMatrix(trans);

			// Draw Borders
		Renderer::drawRect(46.f,-(font->getHeight() / 2) - 2.f, mSize.x() - 96.f + 4.f, 2 * font->getHeight() + 4.f, 0x676767FF);
		Renderer::drawRect(22.f, (1 * mSize.y() / 5) - (font->getHeight() / 2) - 2.f, mSize.x() - 48.f + 4.f, 2 * font->getHeight() + 4.f, 0x676767FF);
		Renderer::drawRect(-2.f, (2 * mSize.y() / 5) - (font->getHeight() / 2) - 2.f, mSize.x() + 4.f, 2 * font->getHeight() + 4.f, 0x676767FF);
		Renderer::drawRect(22.f, (3 * mSize.y() / 5) - (font->getHeight() / 2) - 2.f, mSize.x() - 48.f + 4.f, 2 * font->getHeight() + 4.f, 0x676767FF);
		Renderer::drawRect(46.f, (4 * mSize.y() / 5) -(font->getHeight() / 2) - 2.f, mSize.x() - 96.f + 4.f, 2 * font->getHeight() + 4.f, 0x676767FF);

			// Draw boxes themselves
		Renderer::drawRect(48.f,-(font->getHeight() / 2), mSize.x() - 96.f, 2 * font->getHeight(), 0x00C8F8FF);
		Renderer::drawRect(24.f, (1 * mSize.y() / 5) - (font->getHeight() / 2), mSize.x() - 48.f, 2 * font->getHeight(), 0x008DB1FF);
		Renderer::drawRect(0.f, (2 * mSize.y() / 5) - (font->getHeight() / 2), mSize.x(), 2 * font->getHeight(), 0x006E8CFF);
		Renderer::drawRect(24.f, (3 * mSize.y() / 5) - (font->getHeight() / 2), mSize.x() - 48.f, 2 * font->getHeight(), 0x008DB1FF);
		Renderer::drawRect(48.f, (4 * mSize.y() / 5) -(font->getHeight() / 2), mSize.x() - 96.f, 2 * font->getHeight(), 0x00C8F8FF);
		
	}

	// clip to inside margins
	Eigen::Vector3f dim(mSize.x(), mSize.y(), 0);
	dim = trans * dim - trans.translation();
	Renderer::pushClipRect(Eigen::Vector2i((int)(trans.translation().x() + mHorizontalMargin), (int)trans.translation().y()), 
		Eigen::Vector2i((int)(dim.x() - mHorizontalMargin*2 - 48.f), (int)dim.y()));


		// Start edits

		// First entry. If we are at position 0 or 1, get the last one or two games
	if (mCursor == 0) {
		drawText(size() - 2, 48.f, 0, font, trans);
	}
	else if (mCursor == 1) {
		drawText(size() - 1, 48.f, 0, font, trans);
	}
	else {
		drawText(mCursor - 2, 48.f, 0, font, trans); 
	}

		// Second entry
	if (mCursor == 0) {
		drawText(size() - 1, 24.f, 1 * mSize.y() / 5, font, trans);
	}
	else {
		drawText(mCursor - 1, 24.f, 1 * mSize.y() / 5, font, trans);
	}

		// Third entry
	drawText(mCursor, 0.f, 2 * mSize.y() / 5, font, trans);

		// Fourth entry
	if (mCursor == size() - 1) {
		drawText(0, 24.f, 3 * mSize.y() / 5, font, trans);
	}
	else {
		drawText(mCursor + 1, 24.f, 3 * mSize.y() / 5, font, trans);
	}

		// Fifth entry
	if (mCursor == size() - 2) {
		drawText(0, 48.f, 4 * mSize.y() / 5, font, trans);
	}
	else if (mCursor == size() - 1) {
		drawText(1, 48.f, 4 * mSize.y() / 5, font, trans);
	}
	else {
		drawText(mCursor + 2, 48.f, 4 * mSize.y() / 5, font, trans); 
	}
		
		// End edits
	


	/*for(int i = startEntry; i < listCutoff; i++)
	{
		/*typename IList<TextListData, T>::Entry& entry = mEntries.at((unsigned int) i);

		unsigned int color;
		if(mCursor == i && mSelectedColor)
			color = mSelectedColor;
		else
			color = mColors[entry.data.colorId];

		if(!entry.data.textCache)
			entry.data.textCache = std::unique_ptr<TextCache>(font->buildTextCache(mUppercase ? strToUpper(entry.name) : entry.name, 0, 0, 0x000000FF));

		entry.data.textCache->setColor(color);

		Eigen::Vector3f offset(0, y, 0);

		switch(mAlignment)
		{
		case ALIGN_LEFT:
			offset[0] = mHorizontalMargin;
			break;
		case ALIGN_CENTER:
			offset[0] = (mSize.x() - entry.data.textCache->metrics.size.x()) / 2;
			if(offset[0] < 0)
				offset[0] = 0;
			break;
		case ALIGN_RIGHT:
			offset[0] = (mSize.x() - entry.data.textCache->metrics.size.x());
			offset[0] -= mHorizontalMargin;
			if(offset[0] < 0)
				offset[0] = 0;
			break;
		}
		
		if(mCursor == i)
			offset[0] -= mMarqueeOffset;
		
		Eigen::Affine3f drawTrans = trans;
		drawTrans.translate(offset);
		Renderer::setMatrix(drawTrans);

		font->renderTextCache(entry.data.textCache.get());
		
		

		drawText(i, y, font, trans);
		y += entrySize;
	}*/

	Renderer::popClipRect();

	listRenderTitleOverlay(trans);

	GuiComponent::renderChildren(trans);
}

template <typename T>
void TextListComponent<T>::drawText(int item, float xOffset, float yOffset, std::shared_ptr<Font>& font, Eigen::Affine3f& trans) {

	typename IList<TextListData, T>::Entry& entry = mEntries.at(item);

	unsigned int color;
	if(mCursor == item && mSelectedColor)
		color = mSelectedColor;
	else
		color = mColors[entry.data.colorId];

	if(!entry.data.textCache)
		entry.data.textCache = std::unique_ptr<TextCache>(font->buildTextCache(mUppercase ? strToUpper(entry.name) : entry.name, 0, 0, 0x000000FF));

	entry.data.textCache->setColor(color);

	Eigen::Vector3f offset(0, yOffset, 0);

	/*switch(mAlignment)
	{
	case ALIGN_LEFT:*/
		offset[0] = mHorizontalMargin + xOffset;
		/*break;
	case ALIGN_CENTER:
		offset[0] = (mSize.x() - entry.data.textCache->metrics.size.x()) / 2;
		if(offset[0] < 0)
			offset[0] = 0;
		break;
	case ALIGN_RIGHT:
		offset[0] = (mSize.x() - entry.data.textCache->metrics.size.x());
		offset[0] -= mHorizontalMargin;
		if(offset[0] < 0)
			offset[0] = 0;
		break;
	}*/

	if(mCursor == item)
		offset[0] -= mMarqueeOffset;

	Eigen::Affine3f drawTrans = trans;
	drawTrans.translate(offset);
	Renderer::setMatrix(drawTrans);

	font->renderTextCache(entry.data.textCache.get());

}


template <typename T>
bool TextListComponent<T>::input(InputConfig* config, Input input)
{
	if(size() > 0)
	{
		if(input.value != 0)
		{
			if(config->isMappedTo("down", input))
			{
				listInput(1);
				return true;
			}

			if(config->isMappedTo("up", input))
			{
				listInput(-1);
				return true;
			}
			if(config->isMappedTo("pagedown", input))
			{
				listInput(10);
				return true;
			}

			if(config->isMappedTo("pageup", input))
			{
				listInput(-10);
				return true;
			}
		}else{
			if(config->isMappedTo("down", input) || config->isMappedTo("up", input) || 
				config->isMappedTo("pagedown", input) || config->isMappedTo("pageup", input))
			{
				stopScrolling();
			}
		}
	}

	return GuiComponent::input(config, input);
}

template <typename T>
void TextListComponent<T>::update(int deltaTime)
{
	listUpdate(deltaTime);
	if(!isScrolling() && size() > 0)
	{
		//if we're not scrolling and this object's text goes outside our size, marquee it!
		const std::string& text = mEntries.at((unsigned int)mCursor).name;

		Eigen::Vector2f textSize = mFont->sizeText(text);

		//it's long enough to marquee
		if(textSize.x() - mMarqueeOffset > mSize.x() - 12 - (mAlignment != ALIGN_CENTER ? mHorizontalMargin : 0))
		{
			mMarqueeTime += deltaTime;
			while(mMarqueeTime > MARQUEE_SPEED)
			{
				mMarqueeOffset += MARQUEE_RATE;
				mMarqueeTime -= MARQUEE_SPEED;
			}
		}
	}

	GuiComponent::update(deltaTime);
}

//list management stuff
template <typename T>
void TextListComponent<T>::add(const std::string& name, const T& obj, unsigned int color)
{
	assert(color < COLOR_ID_COUNT);

	typename IList<TextListData, T>::Entry entry;
	entry.name = name;
	entry.object = obj;
	entry.data.colorId = color;
	static_cast<IList< TextListData, T >*>(this)->add(entry);
}

template <typename T>
void TextListComponent<T>::onCursorChanged(const CursorState& state)
{
	mMarqueeOffset = 0;
	mMarqueeTime = -MARQUEE_DELAY;

	if(mCursorChangedCallback)
		mCursorChangedCallback(state);
}

template <typename T>
void TextListComponent<T>::applyTheme(const std::shared_ptr<ThemeData>& theme, const std::string& view, const std::string& element, unsigned int properties)
{
	GuiComponent::applyTheme(theme, view, element, properties);

	const ThemeData::ThemeElement* elem = theme->getElement(view, element, "textlist");
	if(!elem)
		return;

	using namespace ThemeFlags;
	if(properties & COLOR)
	{
		if(elem->has("selectorColor"))
			setSelectorColor(elem->get<unsigned int>("selectorColor"));
		if(elem->has("selectedColor"))
			setSelectedColor(elem->get<unsigned int>("selectedColor"));
		if(elem->has("primaryColor"))
			setColor(0, elem->get<unsigned int>("primaryColor"));
		if(elem->has("secondaryColor"))
			setColor(1, elem->get<unsigned int>("secondaryColor"));
	}

	setFont(Font::getFromTheme(elem, properties, mFont));
	
	if(properties & SOUND && elem->has("scrollSound"))
		setSound(Sound::get(elem->get<std::string>("scrollSound")));

	if(properties & ALIGNMENT)
	{
		if(elem->has("alignment"))
		{
			const std::string& str = elem->get<std::string>("alignment");
			if(str == "left")
				setAlignment(ALIGN_LEFT);
			else if(str == "center")
				setAlignment(ALIGN_CENTER);
			else if(str == "right")
				setAlignment(ALIGN_RIGHT);
			else
				LOG(LogError) << "Unknown TextListComponent alignment \"" << str << "\"!";
		}
		if(elem->has("horizontalMargin"))
		{
			mHorizontalMargin = elem->get<float>("horizontalMargin") * (this->mParent ? this->mParent->getSize().x() : (float)Renderer::getScreenWidth());
		}
	}

	if(properties & FORCE_UPPERCASE && elem->has("forceUppercase"))
		setUppercase(elem->get<bool>("forceUppercase"));

	if(properties & LINE_SPACING && elem->has("lineSpacing"))
		setLineSpacing(elem->get<float>("lineSpacing"));
}
