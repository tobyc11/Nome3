#include "MeshMerger.h"
#include "Subdivision.h"

#include <unordered_map>

namespace Nome::Scene
{
DEFINE_META_OBJECT(CMeshMerger)
{
    BindNamedArgument(&CMeshMerger::Level, "sd_level", 0);
}

inline static const float Epsilon = 0.01f;





void CMeshMerger::UpdateEntity()
{
    if (!IsDirty())
        return;
    subdivisionLevel = Level.GetValue(0);


    Super::UpdateEntity();

    // Update is manual, so this entity has a dummy update method


    SetValid(true);
}

void CMeshMerger::Catmull()
{

    if (subdivisionLevel == 0 || MergedMesh.vertices_empty()) {
        return;
    }
    Mesh.clear();
    //OpenMesh::Subdivider::Uniform::CatmullClarkT<CMeshImpl> catmull; // https://www.graphics.rwth-aachen.de/media/openmesh_static/Documentations/OpenMesh-4.0-Documentation/a00020.html
    // Execute 2 subdivision steps
    CMeshImpl otherMesh = MergedMesh;
    //catmull.attach(otherMesh);
    //prepare(otherMesh);

    subdivide(otherMesh, subdivisionLevel, isSharp);
    std::cout << "Apply catmullclark subdivision, may take a few minutes or so" << std::endl;
    //catmull(4);
    //cleanup(otherMesh);
    //catmull.detach();
    //auto tf = dynamic_cast<Scene::CMeshInstance*>(this)->GetSceneTreeNode()->L2WTransform.GetValue(tc::Matrix3x4::IDENTITY); // The transformation matrix is the identity matrix by default
    auto tf = tc::Matrix3x4::IDENTITY;
    // Copy over all the vertices and check for overlapping
    std::unordered_map<CMeshImpl::VertexHandle, CMeshImpl::VertexHandle> vertMap;
    float maxY = -1 * std::numeric_limits<double>::infinity();
    float minY = std::numeric_limits<double>::infinity();
    for (auto vi = otherMesh.vertices_begin(); vi != otherMesh.vertices_end(); ++vi)
    {
        std::cout << vi->idx() << std::endl;
        const auto& posArray = otherMesh.point(*vi);
        Vector3 localPos = Vector3(posArray[0], posArray[1], posArray[2]);
        Vector3 worldPos = tf * localPos;
        maxY = std::max(maxY, worldPos.y);
        minY = std::min(minY, worldPos.y);
    }
    for (auto vi = otherMesh.vertices_begin(); vi != otherMesh.vertices_end();
         ++vi) // Iterate through all the vertices in the mesh (the non-merger mesh, aka the one
        // you're trying copy vertices from)
    {
        std::cout << vi->idx() << std::endl;
        const auto& posArray = otherMesh.point(*vi);
        Vector3 localPos = Vector3(posArray[0], posArray[1],
                                   posArray[2]);
        Vector3 worldPos = tf * localPos;
        /* Dont need since merged nodes have no overlapping vertices
        auto [closestVert, distance] = FindClosestVertex(
            worldPos);
        if (distance < Epsilon)
        {
            vertMap[*vi] = closestVert;
        }*/
        //else
        auto vnew = Mesh.add_vertex({ worldPos.x, worldPos.y + (maxY - minY) + 10, worldPos.z});
        vertMap[*vi] = vnew;
        std::string vName = "v" + std::to_string(VertCount);
        NameToVert.insert({ vName, vnew });
        ++VertCount;

    }

    // Add faces
    for (auto fi = otherMesh.faces_begin(); fi != otherMesh.faces_end();
         ++fi)
    {
        std::cout << fi->idx() << std::endl;
        std::vector<CMeshImpl::VertexHandle> verts;
        for (auto vert : otherMesh.fv_range(*fi))
            verts.emplace_back(vertMap[vert]);


        auto fnew =
            Mesh.add_face(verts);
        std::string fName = "v" + std::to_string(FaceCount);
        NameToFace.insert(
            { fName,
              fnew });
        FaceCount++;
    }
}

void CMeshMerger::MergeClear() {
    MergedMesh.clear();
    Mesh.clear();
}

void CMeshMerger::MergeIn(CMeshInstance& meshInstance)
{
    auto tf = meshInstance.GetSceneTreeNode()->L2WTransform.GetValue(tc::Matrix3x4::IDENTITY); // The transformation matrix is the identity matrix by default
    auto& otherMesh = meshInstance.GetMeshImpl(); // Getting OpeshMesh implementation of a mesh. This allows us to traverse the mesh's vertices/faces

    // Copy over all the vertices and check for overlapping
    std::unordered_map<CMeshImpl::VertexHandle, CMeshImpl::VertexHandle> vertMap;
    for (auto vi = otherMesh.vertices_begin(); vi != otherMesh.vertices_end(); ++vi) // Iterate through all the vertices in the mesh (the non-merger mesh, aka the one you're trying copy vertices from)
    {

        const auto& posArray = otherMesh.point(*vi);
        Vector3 localPos = Vector3(posArray[0], posArray[1], posArray[2]); // localPos is position before transformations (e.g. rotate, translate, etc.)
        Vector3 worldPos = tf * localPos; // worldPos is the actual position you see in the grid, after the transformation (e.g. rotate, translate, etc.)
        auto [closestVert, distance] = FindClosestVertex(worldPos); // Find closest vertex already IN MERGER mesh, not the actual mesh. This is to prevent adding two merger vertices in the same location!
        // As a side note, closestVert is a VertexHandle, which is essentially, a pointer to the actual vertex. OpenMesh is great at working with these handles. You can basically treat them as the vertex themselves.
        if (distance < Epsilon)
        { // this is to check for cases where there is an overlap (two vertices lie in the exact same world space coordinate). We only want to create one merger vertex at this location!
            vertMap[*vi] = closestVert; //just set vi to the closestVert (which is a merger vertex in the same location added in a previous iteration)
            MergedMesh.data(closestVert).set_sharpness(
                std::max(MergedMesh.data(closestVert).sharpness(),otherMesh.data(*vi).sharpness()));
            printf("set sharpness: %f\n", MergedMesh.data(closestVert).sharpness());
        }
        else // Else, we haven't added a vertex at this location yet. So lets add_vertex to the merger mesh.
        {
            auto vnew = MergedMesh.add_vertex({ worldPos.x, worldPos.y, worldPos.z }); // This adds a new vertex. Notice, we are passing in coordinates here, but it actually returns a vertex handle (essentially, a pointer to this vertex.
            vertMap[*vi] = vnew; // Map actual mesh vertex to merged vertex.This dictionary is useful for add face later.
            std::string vName = "v" + std::to_string(VertCount); // we of course need a name for this new vertex handle
            NameToVert.insert({ vName, vnew }); // Add new merged vertex into NameToVert. This is if there wa sa floating point error above so we need to add an entirely new vertex + position ?
            ++VertCount; // VertCount is an attribute for this merger mesh. Starts at 0.
            MergedMesh.data(vnew).set_sharpness(otherMesh.data(*vi).sharpness());
        }
    }

    // Add faces
    for (auto fi = otherMesh.faces_begin(); fi != otherMesh.faces_end(); ++fi) //Iterate through all the faces in the mesh (that is, the non-merger mesh, aka the one you're trying to copy faces from)
    {
        std::vector<CMeshImpl::VertexHandle> verts;
        for (auto vert : otherMesh.fv_range(*fi)) // iterate through all the vertices on this face
            verts.emplace_back(vertMap[vert]); // Add the vertice handles from above. In most cases, it will match the actual mesh's? Unless there is a floating point precision error?
        auto fnew = MergedMesh.add_face(verts); // add_face processes the merger vertex handles and adds the face into the merger mesh (Mesh refers to the merger mesh here)
        std::string fName = "v" + std::to_string(FaceCount);

        NameToFace.insert({ fName, fnew }); // We add a new face in the same location as the actual mesh's face. This means if we adjust the actual mesh's parameters using a slider, you'll see the merger mesh in the actual mesh's original location
        FaceCount++;
    }
    // Add edge property
    for (auto edge : otherMesh.edges()) //Iterate through all the faces in the mesh (that is, the non-merger mesh, aka the one you're trying to copy faces from)
    {
        MergedMesh.data(edge).set_sharpness(std::max(otherMesh.data(edge).sharpness(), MergedMesh.data(edge).sharpness()));
    }
}

//Find closest vertex in current mesh's vertices
std::pair<CMeshImpl::VertexHandle, float> CMeshMerger::FindClosestVertex(const tc::Vector3& pos)
{
    CMeshImpl::VertexHandle result;
    float minDist = std::numeric_limits<float>::max();
    // TODO: linear search for the time being
    for (const auto& v : MergedMesh.vertices())
    {
        const auto& point = MergedMesh.point(v);
        Vector3 pp = Vector3(point[0], point[1], point[2]);
        float dist = pos.DistanceToPoint(pp);
        if (dist < minDist)
        {
            minDist = dist;
            result = v;
        }
    }
    return { result, minDist };
}

bool CMeshMerger::subdivide(CMeshImpl& _m, unsigned int n, bool isSharp)
{

    // Instantiate a Far::TopologyRefiner from the descriptor
    Far::TopologyRefiner * refiner = GetRefiner(_m, isSharp);


    refiner->RefineUniform(Far::TopologyRefiner::UniformOptions(n));

    std::vector<Vertex> vbuffer(refiner->GetNumVerticesTotal());
    Vertex * verts = &vbuffer[0];

    for (auto v_itr = _m.vertices_begin(); v_itr != _m.vertices_end(); ++v_itr) {
        verts[v_itr->idx()].SetPosition(_m.point(*v_itr)[0], _m.point(*v_itr)[1], _m.point(*v_itr)[2]);
    }


    // Interpolate vertex primvar data
    Far::PrimvarRefiner primvarRefiner(*refiner);

    Vertex * src = verts;
    for (int level = 1; level <= n; ++level) {
        Vertex * dst = src + refiner->GetLevel(level-1).GetNumVertices();
        primvarRefiner.Interpolate(level, src, dst);
        src = dst;
    }
    _m.clear();
    { // Output OBJ of the highest level refined -----------
        /// to debug
        Far::TopologyLevel const & refLastLevel = refiner->GetLevel(n);
        for (int i = 0; i <= n; ++i)
        {
            printf("level:%d has %d vertices\n", i, refiner->GetLevel(i).GetNumVertices());
        }
        printf("total number of %d vertices\n", refiner->GetNumVerticesTotal());

        int nverts = refLastLevel.GetNumVertices();
        int nfaces = refLastLevel.GetNumFaces();

        // Print vertex positions
        int firstOfLastVerts = refiner->GetNumVerticesTotal() - nverts;


        printf("============ output vertices======\n");
        for (int vert = 0; vert < nverts; ++vert) {
            float const * pos = verts[vert + firstOfLastVerts].GetPosition();
            _m.add_vertex(CMeshImpl::Point(pos[0], pos[1], pos[2]));
            printf("v %f %f %f\n", pos[0], pos[1], pos[2]);
        }

        // Print faces
        printf("============ output faces======\n");
        for (int face = 0; face < nfaces; ++face) {
            Far::ConstIndexArray fverts = refLastLevel.GetFaceVertices(face);

            // all refined Catmark faces should be quads
            assert(fverts.size()==4);
            _m.add_face(_m.vertex_handle(fverts[0]), _m.vertex_handle(fverts[1]), _m.vertex_handle(fverts[2]), _m.vertex_handle(fverts[3]));

<<<<<<< HEAD
            printf("f ");
            for (int vert=0; vert<fverts.size(); ++vert) {
                printf("%d ", fverts[vert]+1); // OBJ uses 1-based arrays...
            }
            printf("\n");
=======

            _m.add_face(_m.vertex_handle(fverts[0]), _m.vertex_handle(fverts[1]), _m.vertex_handle(fverts[2]), _m.vertex_handle(fverts[3]));
>>>>>>> subdivision

        }
    }


<<<<<<< HEAD


=======
>>>>>>> subdivision
    /*
    for (int i = 0; i < n; i++) {
        // Compute face centroid
        for ( auto fh : _m.faces())
        {
            CMeshImpl::Point centroid;
            _m.calc_face_centroid( fh, centroid);
            _m.property( fp_pos_, fh ) = centroid;
        }

        // Compute position for new (edge-) vertices and store them in the edge property
        for ( auto eh : _m.edges())
            compute_midpoint( _m, eh, _update_points );

        // position updates activated?
        if(_update_points)
        {
            // compute new positions for old vertices
            for ( auto vh : _m.vertices())
                update_vertex( _m, vh );

            // Commit changes in geometry
            for ( auto vh : _m.vertices())
                _m.set_point(vh, _m.property( vp_pos_, vh ) );
        }

        // Split each edge at midpoint stored in edge property ep_pos_;
        // Attention! Creating new edges, hence make sure the loop ends correctly.
        for ( auto eh : _m.edges())
            split_edge( _m, eh );

        // Commit changes in topology and reconsitute consistency
        // Attention! Creating new faces, hence make sure the loop ends correctly.
        for ( auto fh : _m.faces())
            split_face( _m, fh);


#if defined(_DEBUG) || defined(DEBUG)
        // Now we have an consistent mesh!
        assert( OpenMesh::Utils::MeshCheckerT<MeshType>(_m).check() );
#endif
    }
    */

    //_m.update_normals();

    return true;
}

void CMeshMerger::split_face(CMeshImpl& _m, const CMeshImpl::FaceHandle& _fh)
{
    // Since edges already refined (valence*2)
    size_t valence = _m.valence(_fh)/2;

    // new mesh vertex from face centroid
    auto vh = _m.add_vertex(_m.property( fp_pos_, _fh ));

    auto hend = _m.halfedge_handle(_fh);
    auto hh = _m.next_halfedge_handle(hend);

    auto hold = _m.new_edge(_m.to_vertex_handle(hend), vh);

    _m.set_next_halfedge_handle(hend, hold);
    _m.set_face_handle(hold, _fh);

    hold = _m.opposite_halfedge_handle(hold);

    for(size_t i = 1; i < valence; i++)
    {
        CMeshImpl::HalfedgeHandle hnext = _m.next_halfedge_handle(hh);

        CMeshImpl::FaceHandle fnew = _m.new_face();

        _m.set_halfedge_handle(fnew, hh);

        CMeshImpl::HalfedgeHandle hnew = _m.new_edge(_m.to_vertex_handle(hnext), vh);

        _m.set_face_handle(hnew,  fnew);
        _m.set_face_handle(hold,  fnew);
        _m.set_face_handle(hh,    fnew);
        _m.set_face_handle(hnext, fnew);

        _m.set_next_halfedge_handle(hnew, hold);
        _m.set_next_halfedge_handle(hold, hh);
        _m.set_next_halfedge_handle(hh, hnext);
        hh = _m.next_halfedge_handle(hnext);
        _m.set_next_halfedge_handle(hnext, hnew);

        hold = _m.opposite_halfedge_handle(hnew);
    }

    _m.set_next_halfedge_handle(hold, hh);
    _m.set_next_halfedge_handle(hh, hend);
    hh = _m.next_halfedge_handle(hend);
    _m.set_next_halfedge_handle(hend, hh);
    _m.set_next_halfedge_handle(hh, hold);

    _m.set_face_handle(hold, _fh);

    _m.set_halfedge_handle(vh, hold);
}

void CMeshMerger::split_edge(CMeshImpl& _m, const CMeshImpl::EdgeHandle& _eh)
{
    auto heh     = _m.halfedge_handle(_eh, 0);
    auto opp_heh = _m.halfedge_handle(_eh, 1);

    CMeshImpl::HalfedgeHandle new_heh, opp_new_heh, t_heh;
    CMeshImpl::VertexHandle   vh;
    CMeshImpl::VertexHandle   vh1( _m.to_vertex_handle(heh));
    CMeshImpl::Point          zero(0,0,0);

    // new vertex
    vh = _m.new_vertex( zero );
    _m.set_point( vh, _m.property( ep_pos_, _eh ) );

    // Re-link mesh entities
    if (_m.is_boundary(_eh))
    {
        for (t_heh = heh;
             _m.next_halfedge_handle(t_heh) != opp_heh;
             t_heh = _m.opposite_halfedge_handle(_m.next_halfedge_handle(t_heh)))
        {}
    }
    else
    {
        for (t_heh = _m.next_halfedge_handle(opp_heh);
             _m.next_halfedge_handle(t_heh) != opp_heh;
             t_heh = _m.next_halfedge_handle(t_heh) )
        {}
    }

    new_heh     = _m.new_edge(vh, vh1);
    opp_new_heh = _m.opposite_halfedge_handle(new_heh);
    _m.set_vertex_handle( heh, vh );

    _m.set_next_halfedge_handle(t_heh, opp_new_heh);
    _m.set_next_halfedge_handle(new_heh, _m.next_halfedge_handle(heh));
    _m.set_next_halfedge_handle(heh, new_heh);
    _m.set_next_halfedge_handle(opp_new_heh, opp_heh);

    if (_m.face_handle(opp_heh).is_valid())
    {
        _m.set_face_handle(opp_new_heh, _m.face_handle(opp_heh));
        _m.set_halfedge_handle(_m.face_handle(opp_new_heh), opp_new_heh);
    }

    if( _m.face_handle(heh).is_valid())
    {
        _m.set_face_handle( new_heh, _m.face_handle(heh) );
        _m.set_halfedge_handle( _m.face_handle(heh), heh );
    }

    _m.set_halfedge_handle( vh, new_heh);
    _m.set_halfedge_handle( vh1, opp_new_heh );

    // Never forget this, when playing with the topology
    _m.adjust_outgoing_halfedge( vh );
    _m.adjust_outgoing_halfedge( vh1 );
}

void CMeshMerger::compute_midpoint(CMeshImpl& _m, const CMeshImpl::EdgeHandle& _eh, const bool _update_points)
{
    CMeshImpl::HalfedgeHandle heh, opp_heh;

    heh      = _m.halfedge_handle( _eh, 0);
    opp_heh  = _m.halfedge_handle( _eh, 1);

    CMeshImpl::Point pos( _m.point( _m.to_vertex_handle( heh)));

    pos +=  _m.point( _m.to_vertex_handle( opp_heh));

    // boundary edge: just average vertex positions
    // this yields the [1/2 1/2] mask
    if (_m.is_boundary(_eh) || !_update_points)
    {
        pos *= static_cast<float>(0.5);
    }
//  else if (_m.status(_eh).selected() )
//  {
//    pos *= 0.5; // change this
//  }

    else // inner edge: add neighbouring Vertices to sum
        // this yields the [1/16 1/16; 3/8 3/8; 1/16 1/16] mask
    {
        pos += _m.property(fp_pos_, _m.face_handle(heh));
        pos += _m.property(fp_pos_, _m.face_handle(opp_heh));
        pos *= static_cast<float>(0.25);
    }
    _m.property( ep_pos_, _eh ) = pos;
}

void CMeshMerger::update_vertex( CMeshImpl& _m, const CMeshImpl::VertexHandle& _vh)
{
    CMeshImpl::Point pos(0.0,0.0,0.0);

    // TODO boundary, Extraordinary Vertex and  Creased Surfaces
    // see "A Factored Approach to Subdivision Surfaces"
    // http://faculty.cs.tamu.edu/schaefer/research/tutorial.pdf
    // and http://www.cs.utah.edu/~lacewell/subdeval
    if ( _m.is_boundary( _vh))
    {
        pos = _m.point(_vh);
        CMeshImpl::VertexEdgeIter   ve_itr;
        for ( ve_itr = _m.ve_iter( _vh); ve_itr.is_valid(); ++ve_itr)
            if ( _m.is_boundary( *ve_itr))
                pos += _m.property( ep_pos_, *ve_itr);
        pos /= static_cast<typename OpenMesh::vector_traits<typename CMeshImpl::Point>::value_type>(3.0);
    }
    else // inner vertex
    {
        /* For each (non boundary) vertex V, introduce a new vertex whose
           position is F/n + 2E/n + (n-3)V/n where F is the average of
           the new face vertices of all faces adjacent to the old vertex
           V, E is the average of the midpoints of all edges incident
           on the old vertex V, and n is the number of edges incident on
           the vertex.
           */

        /*
        Normal           Vec;
        VertexEdgeIter   ve_itr;
        double           valence(0.0);
        // R = Calculate Valence and sum of edge midpoints
        for ( ve_itr = _m.ve_iter( _vh); ve_itr; ++ve_itr)
        {
          valence+=1.0;
          pos += _m.property(ep_pos_, *ve_itr);
        }
        pos /= valence*valence;
        */

        double valence(0.0);
        CMeshImpl::VOHIter voh_it = _m.voh_iter( _vh );
        for( ; voh_it.is_valid(); ++voh_it )
        {
            pos += _m.point( _m.to_vertex_handle( *voh_it ) );
            valence+=1.0;
        }
        pos /= valence*valence;

        CMeshImpl::VertexFaceIter vf_itr;
        CMeshImpl::Point          Q(0, 0, 0);

        for ( vf_itr = _m.vf_iter( _vh); vf_itr.is_valid(); ++vf_itr) //, neigboring_faces += 1.0 )
        {
            Q += _m.property(fp_pos_, *vf_itr);
        }

        Q /= valence*valence;//neigboring_faces;

        pos += _m.point(_vh) * (valence - 2.0 )/valence + Q;
        //      pos = vector_cast<Vec>(_m.point(_vh));
    }

    _m.property( vp_pos_, _vh ) = pos;
}







}