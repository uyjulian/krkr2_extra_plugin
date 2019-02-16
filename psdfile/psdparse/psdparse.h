#ifndef __psdparse_h__
#define __psdparse_h__

#include <algorithm>
#include <boost/spirit/include/qi.hpp>
#include <boost/spirit/include/phoenix_bind.hpp>
#include <boost/range/iterator_range.hpp>
#include <boost/spirit/include/phoenix_core.hpp>
#include <boost/spirit/include/phoenix_operator.hpp>
#include <boost/spirit/repository/include/qi_advance.hpp>

#include "psddata.h"

namespace psd {

	// �ėp�Ńo�b�t�@�R�s�[
	template <typename Iterator>
	inline void copyToBuffer(uint8_t *buffer, Iterator &start, int size) {
		Iterator end = start;
		std::advance(end, size);
#if defined(_MSC_VER) && _MSC_VER >= 1400
// std::copy �� security warning �}�~
#pragma warning(push) 
#pragma warning(disable:4996) 
#endif 
		std::copy(start, end, buffer);
#if defined(_MSC_VER) && _MSC_VER >= 1400 
#pragma warning(pop) 
#endif 
		start = end;
	}

	template <typename Iterator>
	inline void getShortBE(uint8_t *var8, Iterator &cur) {
		var8[1] = *cur++;
		var8[0] = *cur++;
	}

	template <typename Iterator>
	inline void getShortLE(uint8_t *var8, Iterator &cur) {
		var8[0] = *cur++;
		var8[1] = *cur++;
	}

	template <typename Iterator>
	inline void getLongBE(uint8_t *var8, Iterator &cur) {
		var8[3] = *cur++;
		var8[2] = *cur++;
		var8[1] = *cur++;
		var8[0] = *cur++;
	}

	template <typename Iterator>
	inline void getLongLE(uint8_t *var8, Iterator &cur) {
		var8[0] = *cur++;
		var8[1] = *cur++;
		var8[2] = *cur++;
		var8[3] = *cur++;
	}

	template <typename Iterator>
	inline void getLongLongBE(uint8_t *var8, Iterator &cur) {
		var8[7] = *cur++;		var8[6] = *cur++;
		var8[5] = *cur++;		var8[4] = *cur++;
		var8[3] = *cur++;		var8[2] = *cur++;
		var8[1] = *cur++;		var8[0] = *cur++;
	}

	template <typename Iterator>
	inline void getLongLongLE(uint8_t *var8, Iterator &cur) {
		var8[0] = *cur++;		var8[1] = *cur++;
		var8[2] = *cur++;		var8[3] = *cur++;
		var8[4] = *cur++;		var8[5] = *cur++;
		var8[6] = *cur++;		var8[7] = *cur++;
	}
	
	// �ėp�C�e���[�^�Q��
	template <typename Iterator>
	class IteratorData : public IteratorBase {
	public:
		typedef boost::iterator_range<Iterator> irange;
		IteratorData(irange range) : range(range) {
			init();
		};
		IteratorBase *clone() {
			return new IteratorData(range);
		}
    IteratorBase *cloneOffset(int offset) {
      IteratorData *id = new IteratorData(range);
      if (offset != 0) {
        id->range.advance_begin(offset);
        id->init();
      }
      return id;
    }
		virtual void init() {
			cur = range.begin();
		}
		virtual int getCh() {
			if (eoi()) {
				return -1;
			}
			return *cur++;
		}
		virtual int16_t getInt16(bool convToNative) {
      if (std::distance(cur, range.end()) < 2) {
        return -1;
      }
      union {
        uint8_t var8[2];
        int16_t var16;
      } ret;
#ifdef BOOST_LITTLE_ENDIAN
      bool swap = convToNative;
#else
      bool swap = !convToNative;
#endif
      if (swap) {
				getShortBE(ret.var8, cur);
      } else {
				getShortLE(ret.var8, cur);
      }
      return ret.var16;
		}
		virtual int32_t getInt32(bool convToNative) {
      if (std::distance(cur, range.end()) < 4) {
        return -1;
      }
      union {
        uint8_t var8[4];
        int32_t var32;
      } ret;
#ifdef BOOST_LITTLE_ENDIAN
      bool swap = convToNative;
#else
      bool swap = !convToNative;
#endif
      if (swap) {
				getLongBE(ret.var8, cur);
      } else {
				getLongLE(ret.var8, cur);
      }
      return ret.var32;
		}
		virtual int64_t getInt64(bool convToNative) {
      if (std::distance(cur, range.end()) < 8) {
        return -1;
      }
      union {
        uint8_t var8[8];
        int64_t var64;
      } ret;
#ifdef BOOST_LITTLE_ENDIAN
      bool swap = convToNative;
#else
      bool swap = !convToNative;
#endif
      if (swap) {
				getLongLongBE(ret.var8, cur);
      } else {
				getLongLongLE(ret.var8, cur);
      }
      return ret.var64;
		}
		virtual int getData(void *buffer, int size) {
			if (eoi()) {
				return 0;
			}
			int remain = (int)std::distance(cur, range.end());
			if (size > remain) {
				size = remain;
			}
			
			copyToBuffer((uint8_t*)buffer, cur, size);
			
			return size;
		}
		virtual bool eoi() {
			return cur == range.end();
		}
    virtual void getUnicodeString(std::wstring &str, bool convToNative=true) {
      int size = getInt32(true);
      str.clear();
      for (int i = 0; i < size; i++) {
        str.push_back((wchar_t)getInt16(convToNative));
      }
    }
    virtual int size() {
      return std::distance(range.begin(), range.end());
    }
    virtual int rest() {
      return std::distance(cur, range.end());
    }
    virtual void advance(int size) {
			if (!eoi()) {
        int remain = (int)std::distance(cur, range.end());
        if (size > remain) {
          size = remain;
        }
        std::advance(cur, size);
      }
    }
	private:
		irange range;
		Iterator cur;
	};

	namespace spirit = boost::spirit;
	namespace qi     = spirit::qi;
	namespace phx    = boost::phoenix;
	namespace repos  = boost::spirit::repository;

	template <typename Iterator>
	struct HeaderParser : qi::grammar<Iterator> {
		HeaderParser(Header &data) : HeaderParser::base_type(start), data(data) {
			start =
				(qi::lit("8BPS") >>
				 qi::big_word [phx::bind(&Header::version, data) = qi::_1] >>
         repos::qi::advance(6) >>
				 qi::big_word [phx::bind(&Header::channels, data) = qi::_1] >>
				 qi::big_dword[phx::bind(&Header::height,  data) = qi::_1] >>
				 qi::big_dword[phx::bind(&Header::width,   data) = qi::_1] >>
				 qi::big_word [phx::bind(&Header::depth,   data) = qi::_1] >>
				 qi::big_word [phx::bind(&Header::mode,    data) = qi::_1]
				 );
		}
		qi::rule<Iterator> start;
		Header &data;
	};
	
	/**
	 * �C���[�W���\�[�X�p�[�T
	 */
	template <typename Iterator>
	struct ImageResourceParser : qi::grammar<Iterator> {

		typedef boost::iterator_range<Iterator> irange;

		ImageResourceParser(Data &data) : ImageResourceParser::base_type(start), data(data) {
			// �J�b�R���Ŏ����g���Ȃ�advance�p�Ɏ��O�Ƀp�f�B���O�����v�Z���Ă������߂̈ꎞ�ϐ�
			int paddedSize = 0;

			// �C���[�W���\�[�X�P��
			anImageResource =
				(qi::lit("8BIM") >>                                  // signature
				 qi::big_word >>                                     // resource ID
				 qi::byte_ [qi::_a = qi::_1] >>                      // name size
				 spirit::repeat(qi::_a)[qi::char_] >>                // name
				 spirit::repeat((qi::_a + 1)%2)[qi::byte_] >>        // �p�f�B���O�ǂݔ�΂�
         qi::big_dword [qi::_a = qi::_1] [phx::ref(paddedSize) = (qi::_a+1)/2*2] >> // �T�C�Y
         qi::raw[repos::qi::advance(phx::ref(paddedSize))]   // �f�[�^(�p�f�B���O�܂�)
				 )[phx::bind(&ImageResourceParser::addImageResource, this, qi::_1, qi::_3, qi::_5, qi::_6)];
			start = *anImageResource;
		}
		qi::rule<Iterator, qi::locals<boost::uint32_t>> anImageResource;
		qi::rule<Iterator> start;
		Data &data;

		// �C���[�W���\�[�X����ǉ�����
		void addImageResource(uint16_t id, std::vector<char> &name, int size, irange range) {
			std::string strname;
			if (name.size() > 0) {
				strname.assign(&name[0], name.size());
			}
			ImageResourceInfo info(id, strname, size, new IteratorData<Iterator>(range));
			data.imageResourceList.push_back(info);
		}
	};

	/**
	 * ���C�� extra data �p�[�T
	 */
	template <typename Iterator>
	struct LayerMaskParser : qi::grammar<Iterator> {
		
		typedef boost::iterator_range<Iterator> irange;
		
		LayerMaskParser(LayerMask &data, int size) : LayerMaskParser::base_type(start), data(data) {
			start =
				 qi::big_dword [phx::bind(&LayerMask::top, data) = qi::_1] >>
				 qi::big_dword [phx::bind(&LayerMask::left, data) = qi::_1] >>
				 qi::big_dword [phx::bind(&LayerMask::bottom, data) = qi::_1] >>
				 qi::big_dword [phx::bind(&LayerMask::right, data) = qi::_1] >>
				 qi::byte_ [phx::bind(&LayerMask::defaultColor, data) = qi::_1] >>
				 qi::byte_ [phx::bind(&LayerMask::flags, data) = qi::_1] >>
				 qi::byte_;

			if (size > 20) {
				start = start.copy() >>
					qi::byte_ [phx::bind(&LayerMask::realFlags, data) = qi::_1] >>
					qi::byte_ [phx::bind(&LayerMask::realUserMaskBackground, data) = qi::_1] >>
					qi::big_dword [phx::bind(&LayerMask::enclosingTop, data) = qi::_1] >>
					qi::big_dword [phx::bind(&LayerMask::enclosingLeft, data) = qi::_1] >>
					qi::big_dword [phx::bind(&LayerMask::enclosingBottom, data) = qi::_1] >>
					qi::big_dword [phx::bind(&LayerMask::enclosingRight, data) = qi::_1];
			}
		}
		qi::rule<Iterator> start;
		LayerMask &data;
	};

	/**
	 * ���C�� blending range �p�[�T
	 */
	template <typename Iterator>
	struct LayerBlendingRangeParser : qi::grammar<Iterator> {
		LayerBlendingRangeParser(LayerBlendingRange &data) : LayerBlendingRangeParser::base_type(start), data(data) {
			channel =
				(qi::big_dword >> qi::big_dword)
				[phx::bind(&LayerBlendingRangeParser::addChannel, this, qi::_1, qi::_2)];
			start =
				 qi::big_dword [phx::bind(&LayerBlendingRange::grayBlendSource, data) = qi::_1] >>
				 qi::big_dword [phx::bind(&LayerBlendingRange::grayBlendDest, data) = qi::_1] >>
				 *channel;
		}
		void addChannel(int source, int dest) {
			LayerBlendingChannel ch = { source, dest };
			data.channels.push_back(ch);
		}
		qi::rule<Iterator> channel;
		qi::rule<Iterator> start;
		LayerBlendingRange &data;
	};
	
	/**
	 * ���C�� extra data �p�[�T
	 */
	template <typename Iterator>
	struct LayerExtraDataParser : qi::grammar<Iterator> {
		
		typedef boost::iterator_range<Iterator> irange;
		
		LayerExtraDataParser(LayerExtraData &data) : LayerExtraDataParser::base_type(start), data(data) {

			layerMask =
				(qi::big_dword[qi::_a = qi::_1] >>
				 qi::raw[repos::qi::advance(qi::_a)]
				 )[phx::bind(&LayerExtraDataParser::setLayerMask, this, qi::_1, qi::_2)];

			layerBlendingRange = 
				(qi::big_dword[qi::_a = qi::_1] >>
				 qi::raw[repos::qi::advance(qi::_a)]
				 )[phx::bind(&LayerExtraDataParser::setLayerBlendingRange, this, qi::_1, qi::_2)];

			layerName =
				(qi::byte_ [qi::_a = qi::_1] >>            // name size
				 spirit::repeat(qi::_a)[qi::char_] [phx::bind(&LayerExtraDataParser::setLayerName, this, qi::_1)] >>
				 spirit::repeat((4-(qi::_a+1)&3)&3)[qi::byte_] // padding
				 );

			// �ǉ����C�����
			additional =
				((qi::lit("8BIM")[qi::_a=0] | qi::lit("8B64")[qi::_a=1]) >>
				 qi::big_dword >> // key
				 qi::big_dword[qi::_b = qi::_1] >> // data length
         qi::raw[repos::qi::advance(qi::_b)] // data
				 )[phx::bind(&LayerExtraDataParser::addAdditional, this, qi::_a, qi::_1, qi::_2, qi::_3)];

			start =
				(layerMask >>
				 layerBlendingRange >>
				 layerName >>
         *additional
				 );
		}
		qi::rule<Iterator, qi::locals<boost::uint32_t>> layerMask;
		qi::rule<Iterator, qi::locals<boost::uint32_t>> layerBlendingRange;
		qi::rule<Iterator, qi::locals<boost::uint32_t>> layerName;
		qi::rule<Iterator, qi::locals<int, boost::uint32_t>> additional;
		qi::rule<Iterator> start;
		
		void setLayerMask(int size, irange range) {
			if (size > 0) {
				LayerMaskParser<Iterator> parser(data.layerMask, size);
				bool r = qi::parse(range.begin(), range.end(), parser);
        data.layerMask.width  = data.layerMask.right - data.layerMask.left;
        data.layerMask.height = data.layerMask.bottom - data.layerMask.top;
      } else {
        std::memset(&data.layerMask, 0, sizeof(LayerMask));
      }
		}

		void setLayerBlendingRange(int size, irange range) {
			if (size > 0) {
				LayerBlendingRangeParser<Iterator> parser(data.layerBlendingRange);
				bool r = qi::parse(range.begin(), range.end(), parser);
			} else {
        std::memset(&data.layerBlendingRange, 0, sizeof(LayerBlendingRange));
      }
		}
		
		void setLayerName(std::vector<char> &name) {
			if (name.size() > 0) {
				data.layerName.assign(&name[0], name.size());
			}
		}

    void addAdditional(int sigType, int key, int size, irange range) {
      data.additionalLayers.push_back(AdditionalLayerInfo(sigType, key, size, new IteratorData<Iterator>(range)));
		}

		LayerExtraData &data;
	};
	
	/**
	 * ���C�����p�[�T
	 */
	template <typename Iterator>
	struct LayerInfoParser : qi::grammar<Iterator> {

		typedef boost::iterator_range<Iterator> irange;

		LayerInfoParser(Data &data) : LayerInfoParser::base_type(start), data(data) {

			// �`�����l�����
			channelInfo =
				(qi::big_word >> // id
				 qi::big_dword  // length
				 )[phx::bind(&LayerInfoParser::addChannel, this, qi::_1, qi::_2)];

			// �G�N�X�g���f�[�^�p����
			extraData =
			    (qi::big_dword[qi::_a = qi::_1] >> // extra data size
				 qi::raw[repos::qi::advance(qi::_a)]
				 )[phx::bind(&LayerInfoParser::setExtraData, this, qi::_1, qi::_2)];
			
			// ���C�����
			layerRecord =
				(qi::eps [phx::bind(&LayerInfoParser::addLayer, this)] >> 
				 qi::big_dword >> // top
				 qi::big_dword >> // left
				 qi::big_dword >> // bottom
				 qi::big_dword    // right
				 )[phx::bind(&LayerInfoParser::setLayerSize, this, qi::_1, qi::_2, qi::_3, qi::_4)] >>
				 qi::big_word[qi::_a = qi::_1]  >> // channel nums
				 spirit::repeat(qi::_a)[channelInfo] >> // channel information
				 qi::lit("8BIM") >> // signature
				(qi::big_dword >> // blend mode key
				 qi::byte_     >> // opacity
				 qi::byte_     >> // clipping
				 qi::byte_        // flag
				 )[phx::bind(&LayerInfoParser::setLayerData, this, qi::_1, qi::_2, qi::_3, qi::_4)] >>
				 qi::byte_     >> // filler(zero)
				 extraData
				 ;
			// �p�[�X�J�n��
			start =
				(qi::big_word[phx::bind(&LayerInfoParser::setLayerCount, this, qi::_1)] >>
				 layerRecords >>
				 qi::raw[*qi::byte_][phx::bind(&LayerInfoParser::setChannelImageData, this, qi::_1)]
				 );
		}

		// ���C�����w��
		void setLayerCount(boost::int16_t count) {
			data.mergedAlpha = count < 0;
			if (count == 0) {
				layerRecords = qi::eps;
			} else {
				layerRecords = spirit::repeat(abs(count))[layerRecord];
			}
		}

		void addChannel(int16_t id, int length) {
			data.layerList.back().channels.push_back(ChannelInfo(id, length));
		}

		void addLayer() {
			data.layerList.push_back(LayerInfo());
		}

		// �`�����l���C���[�W�f�[�^�̏ꏊ���L�^
		void setExtraData(int size, irange range) {
			if (size > 0) {
				LayerExtraDataParser<Iterator> parser(data.layerList.back().extraData);
				bool r = qi::parse(range.begin(), range.end(), parser);
			}
		}

		// ���C����{����ݒ�
		void setLayerSize(int top, int left, int bottom, int right) {
			LayerInfo &info = data.layerList.back();
			info.top = top;
			info.left = left;
			info.bottom = bottom;
			info.right = right;
      info.width = right - left;
      info.height = bottom - top;
		}

		// ���C���̂��̑��̃f�[�^��ݒ�
		void setLayerData(int blendModeKey, int opacity, int clipping, int flag) {
			LayerInfo &info = data.layerList.back();
			info.blendModeKey = blendModeKey;
      info.blendMode = blendKeyToMode(blendModeKey);
			info.opacity = opacity;
			info.clipping = clipping;
			info.flag = flag;
		}

		// �`�����l���C���[�W�f�[�^�̏ꏊ���L�^
		void setChannelImageData(irange range) {
			data.channelImageData = new IteratorData<Iterator>(range);
		}
		
		qi::rule<Iterator> channelInfo;
		qi::rule<Iterator, qi::locals<boost::uint32_t>> extraData;
		qi::rule<Iterator, qi::locals<boost::uint32_t>> layerRecord;
		qi::rule<Iterator> layerRecords;
		qi::rule<Iterator> start;
		
		Data &data;
	};

	// Global layer mask info parser
	template <typename Iterator>
	struct GlobalLayerMaskInfoParser : qi::grammar<Iterator> {
		GlobalLayerMaskInfoParser(GlobalLayerMaskInfo &data) : GlobalLayerMaskInfoParser::base_type(start), data(data) {
			start =
				(qi::big_word[phx::bind(&GlobalLayerMaskInfo::overlayColorSpace, data) = qi::_1] >>
				 qi::big_word[phx::bind(&GlobalLayerMaskInfo::color1,  data) = qi::_1] >>
				 qi::big_word[phx::bind(&GlobalLayerMaskInfo::color2,  data) = qi::_1] >> 
				 qi::big_word[phx::bind(&GlobalLayerMaskInfo::color3,  data) = qi::_1] >> 
				 qi::big_word[phx::bind(&GlobalLayerMaskInfo::color4,  data) = qi::_1] >> 
				 qi::big_word[phx::bind(&GlobalLayerMaskInfo::opacity, data) = qi::_1] >>
				 qi::byte_   [phx::bind(&GlobalLayerMaskInfo::kind,    data) = qi::_1] >>
				 *qi::byte_
				 );
		}
		qi::rule<Iterator> start;
		GlobalLayerMaskInfo &data;
	};
	
	/**
	 * layer and mask parser
	 */
	template <typename Iterator>
	struct LayerAndMaskParser : qi::grammar<Iterator> {

		typedef boost::iterator_range<Iterator> irange;

		LayerAndMaskParser(Data &data) : LayerAndMaskParser::base_type(start), data(data) {

			layerInfo =
				(qi::big_dword[qi::_a = qi::_1] >> // size
				 qi::raw[repos::qi::advance(qi::_a)] // data
				 )[phx::bind(&LayerAndMaskParser::setLayerInfo, this, qi::_1, qi::_2)];

			globalLayerMaskInfo =
				(qi::big_dword[qi::_a = qi::_1] >> // size
				 qi::raw[repos::qi::advance(qi::_a)] // data
				 )[phx::bind(&LayerAndMaskParser::setGlobalLayerMaskInfo, this, qi::_1, qi::_2)];

      layerInfo2 =
        qi::lit("8BIM") >>
        (qi::lit("Lr16") | qi::lit("Lr32")) >> // TODO Txt2, Patt, Pat2
        layerInfo;

			start =
				layerInfo >>    // ���C�����
				globalLayerMaskInfo >> // �O���[�o�����C���}�X�N
         -layerInfo2
				;
		}

		qi::rule<Iterator, qi::locals<boost::uint32_t>> layerInfo;
		qi::rule<Iterator, qi::locals<boost::uint32_t>> layerInfo2;
		qi::rule<Iterator, qi::locals<boost::uint32_t>> globalLayerMaskInfo;
		qi::rule<Iterator> start;

		void setLayerInfo(int size, irange range) {
			if (size > 0) {
				LayerInfoParser<Iterator> parser(data);
				bool r = qi::parse(range.begin(), range.end(), parser);
			}
		}

		void setGlobalLayerMaskInfo(int size, irange range) {
			if (size > 0) {
				GlobalLayerMaskInfoParser<Iterator> parser(data.globalLayerMaskInfo);
				bool r = qi::parse(range.begin(), range.end(), parser);
			}
		}
		
		Data &data;
	};

	/**
	 * PSD Parser
	 */
	template <typename Iterator>
	struct Parser : qi::grammar<Iterator> {

		typedef boost::iterator_range<Iterator> irange;

		Parser(Data &data)
			 : Parser::base_type(start), data(data), headerParser(data.header) {

			// �J���[���[�h���
			colorMode =
				(qi::big_dword[qi::_a = qi::_1] >>          // size
				 qi::raw[repos::qi::advance(qi::_a)] // data
				 )[phx::bind(&Parser::setColorModeData, this, qi::_1, qi::_2)];

			// �C���[�W���\�[�X����
			imageResource =
				(qi::big_dword[qi::_a = qi::_1] >>          // size
				 qi::raw[repos::qi::advance(qi::_a)]        // data
				)[phx::bind(&Parser::setImageResource, this, qi::_1, qi::_2)];
			
			// ���C���ƃ}�X�N
			layerAndMask =
				(qi::big_dword[qi::_a = qi::_1] >>      // size
         qi::raw[repos::qi::advance(qi::_a)] // data
				)[phx::bind(&Parser::setLayerAndMask, this, qi::_1, qi::_2)];

			// �摜�f�[�^
			imageData = qi::raw[+qi::byte_][phx::bind(&Parser::setImageData, this, qi::_1)];

			start =
				headerParser >>
				colorMode >>
				imageResource >>
				layerAndMask >>
        -imageData >>
        qi::eoi;
		}

		HeaderParser<Iterator> headerParser;
		
		// rules
		qi::rule<Iterator, qi::locals<boost::uint32_t>> colorMode;
		qi::rule<Iterator, qi::locals<boost::uint32_t>> imageResource;
		qi::rule<Iterator, qi::locals<boost::uint32_t>> layerAndMask;
		qi::rule<Iterator> imageData;
		qi::rule<Iterator> start;

		// �J���[���[�h�f�[�^��o�^
		void setColorModeData(int size, irange range) {
			data.colorModeSize = size;
			data.colorModeIterator = new IteratorData<Iterator>(range);
		}
		
		/**
		 * �C���[�W���\�[�X�����
		 * @param size �T�C�Y
		 * @param range �f�[�^�̈�
		 */
		void setImageResource(int size, irange range) {
			if (size > 0) {
				ImageResourceParser<Iterator> parser(data);
				bool r = qi::parse(range.begin(), range.end(), parser);
			}
		}

		/**
		 * ���C���E�}�X�N�������
		 * @param size �T�C�Y
		 * @param �f�[�^�̈�
		 */
		void setLayerAndMask(int size, irange range) {
			if (size > 0) {
				LayerAndMaskParser<Iterator> parser(data);
				bool r = qi::parse(range.begin(), range.end(), parser);
			}	
		}

		// �����ς݉摜�f�[�^�̐擪�ʒu��o�^
		void setImageData(irange range) {
			data.imageData = new IteratorData<Iterator>(range);
		}
		
		Data &data;
	};
}

#endif
