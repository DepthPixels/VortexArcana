namespace Vortex {
	class Texture2D {
	public:
		// Texture ID.
		unsigned int ID;
		// Texture Dimensions.
		unsigned int width, height;

		// Texture Format (RGB, RGBA, etc.)
		unsigned int internalFormat;
		unsigned int imageFormat;

		// Wrapping and Filtering.
		unsigned int wrapS;
		unsigned int wrapT;
		unsigned int filterMin;
		unsigned int filterMax;

		Texture2D();

		void Generate(unsigned int width, unsigned int height, unsigned char* data);

		void Bind() const;
	};
}